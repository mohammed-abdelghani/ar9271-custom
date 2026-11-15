# 🎯 السبب الجذري للـ WMI wedge — تم حسمه بالدليل وإصلاحه وتشغيله

> **التاريخ:** 2026-11-14/15 | **الحالة:** السبب الجذري مثبت بـ usbmon + bisect حي، مُصلح، ومُثبت بـ test suite كامل (50 PASS / صفر WMI timeouts)
> **هذا المستند يحسم العطل العشوائي المفتوح الموثق في docs/10 وdocs/12** ("يموت بعد ~2 رد مخصص").

---

## 1️⃣ السبب الجذري (مثبت بالدليل)

**أي حزمة WMI control على EP3 IN طولها الكلي يصل حدود USB max packet (64 بايت) تعلق USB controller في الشريحة بعد إرسالها.**

### الدليل (usbmon على الـ wedge المتكرر):

```
+1016996 SUB-OUT ep4  0026 034e        <- أمر rssi_hist (48 bins، reset=0)
+1017351 C   o  ep4                    <- الأمر وصل
+1017373 RCV-IN  ep3  0026 034e 64B    <- الرد رجع كامل وصحيح (cmd+seq متطابقين!)
+1017512 SUB-OUT ep4  0026 034f        <- الأمر التالي (بعد 124µs)
                                       <- لا "C o" أبداً: الهدف وقف ACK
بعدها: 1401 × RCV-IN ep3 status=-75 (EOVERFLOW) طول 0 كل ~2ms
```

- الرد المشكوك فيه كان **64 بايت على السلك بالضبط**: 8B HTC hdr + 4B WMI hdr + 48B bins + 4B credit-report trailer.
- الحزمة 64B تُسلَّم للـ host **بنجاح** — ثم الـ controller يعلق: EP4 OUT يتوقف عن ACK، وEP3 IN يبث حزم >64B عند كل poll → EOVERFLOW لا نهائي.
- كل أوامر الـ init (842+ أمر، ردود ≤48B) نجحت بصفر timeouts — الحجم هو المتغير الحاسم لا العدد.
- بعد الـ wedge: أي محاولة استيقاظ ("Failed to wakeup in 500us") → الـ USB يعيد التعداد → **الفيرموير يعيد الإقلاع** (self-heal عبر reboot). وإذا وقع الـ wedge والـ CPU علق تماماً → authorized toggle لا يكفي → **فك فيزيائي** (مطابق للقاعدة الذهبية 5 في docs/10).

### الثلاثة أوامر فوق الحد (كلها من نفس السبب):

| الأمر | payload | على السلك bare | مع trailer | المصير |
|---|---|---|---|---|
| rssi_hist (48 bins) | 48B | 60B | **64B** | wedge عند وجود trailer (كان دائماً موجوداً) |
| chan_survey (nf s16 قديم) | 52B | **64B** | 68B | wedge دائم |
| netflow (entry 12B فعلياً رغم كومنت "10B") | 52B | **64B** | 68B | wedge دائم |

- التفسير التاريخي "سقف الرد 64B payload / WMI_AllocEvent يفشل فوقها" (docs/10 قاعدة 1) كان **غير دقيق**: المطلوب أن تبقى **الحزمة الكلية** (HTC+WMI+payload+trailer) **أقل من 64B** — الـ 64B-تماماً هو الحد المعلِّق.

## 2️⃣ الإصلاحات المطبقة (كلها مثبتة حياً)

| # | الملف | التعديل | الإثبات |
|---|---|---|---|
| Fix 1 | fw `htc/htc.c` `_HTC_SendMsg` | cap للـ credit-report trailer: حزم الـ control pipe (EP3) ≤ `HTC_EP3_SAFE_PACKET`(60B) — الأحوال credit تتأجل للحزمة التالية | rssi_hist 48-bin رد 60B بلا trailer: نجح + ping حي بعده |
| Fix 2 | fw+driver `wmi.h` chan_survey | nf: s16→**s8** (clamp في الـ handler) → 38B payload = 46B wire | chan_survey يرد ويبقى حياً |
| Fix 3 | fw+driver netflow | الـ entry رجع 10B كما قصد الكاتب (bytes بوحدات 256B saturating) → 44B payload = 56B wire | netflow يرد ويبقى حياً |
| Fix 4 | driver `htc_drv_debug.c` | `dbg_reg_read/write/rmw_verify`: قراءة/كتابة ريجيستر مُتحققة + **ps_wakeup/ps_restore** حول الوصول + retry للـ PHY commit latency | nf حقيقية (-43 dBm مرتبطة / flagged عند النوم)، firstep 0/1/2 → 0/2/4 verified |
| Fix 5 | driver `read_file_ack_timeout` | query بقيمة 0 بدل 0xFF — **القراءة القديمة كانت تدمّرية**: 0xFF داخل مدى الكتابة فكانت كل قراءة تعيد كتابة 255 فوق أي قيمة | write 200 → readback 200 (PS off) |
| Fix 6 | driver `wmi_diag` | last_fail cmd/seq/errno + last_ok_rsp + rsp_count (والحسابات تتطابق: issued−timeouts=replies عبر الـ u16 wrap) | «issued 85629 − 206 timeouts = 85423 replies» |
| Fix 7 | fw+driver thermal | cap=0xFF = **uncapped** فعلاً (كان unreachable — query marker كان يعترضه)، query للـ cap = 0x100 | cap 48 → effective 48 → cap 255 → uncapped ✓ |
| Fix 8 | driver blackbox names | off-by-one (نوع 1=ani-step … 8=ack-to، وأُضيف "(type-0)") | "freq-note: 2472" بدل "spectral: 2472" |
| Fix 9 | driver nf/firstep reads | لا مزيد من sentinels: فشل WMI يرجع errno حقيقي للقارئ | أثناء wedge مصطنع: firstep أظهر 63 (sentinel) بينما ping ميت — الآن يظهر خطأ صريح |

**ميزانية الذاكرة بعد الإصلاحات:** `.text` = 0x8A8B (35,467B) — هامش iram ≈ 3,709B، هامش dram ≈ 2,208B — سليم.

## 3️⃣ نتائج الاختبار الكامل النهائي (test-all.sh، بعد كل الإصلاحات)

- **صفر WMI timeouts** عبر الأقسام كلها (`cmd_timeouts=0->0` في كل health check)، القناة عاشت من الأول للآخر.
- كل الأقسام 1–16 تستجيب بقيم حية: ping round trip ✓، rate_usage ✓، **rssi_hist (القاتل القديم) ✓**، ani_auto ✓، macfilter add/verify/clear ✓، txpower=60 ✓، baw 32↔64 ✓، rc_profile 1↔2 ✓، **netflow ✓**، **chan_survey ✓**، auto_spectral on/off ✓، blackbox ✓، **firstep write→readback verified (2/2) ✓**، **thermal cap 48 → effective 48 ✓ + uncapped ✓**.
- الـ FAIL الثلاثة المتعمدة (صدق لا تساهل):
  1. **ack_timeout readback لا يحتفظ** (16111): الريجيستر 0x8014 على K2 تعيد mac80211 كتابته عند كل channel config، وROM يعيد ظل ريجسترات MAC عبر sleep/wake. مسار WMI نفسه مثبت (blackbox يسجل القيمة المستلمة). **غير محسوم من طرف إلى طرف — مؤجل.**
  2. **nf غير منطقية (-236) على راديو idle غير مرتبط**: قيمة حقيقية من الريجيستر لكن NF غير معاير بدون beacon sampling. عند الارتباط القيمة حقيقية (-43 dBm مُثبتة). السكربت يرفض عرضها كقياس.

## 4️⃣ اكتشافات جانبية مهمة (مثبتة أثناء الجلسة)

1. **`CONFIG_ATH_DEBUG` غير مفعّل في الكيرنل** → كل رسائل `ath_dbg` (ومنها "Timeout waiting for WMI command" و"REGISTER READ FAILED") **لا تُطبع أبداً** — كتابة debug mask (0x2000) في deploy.sh بلا أثر. الاعتماد أصبح على `wmi_diag` (لا يتأثر). لو أردت رسائل dmesg: أضف `-DCONFIG_ATH_DEBUG` لـ `ath9k-htc.kbuild` ccflags.
2. **`/etc/modprobe.d/ath9k_htc-stability.conf` لم يكن موجوداً** رغم docs/11 → بعد أي reboot كان الفيرموير الأصلي 1.4.0 هو المحمّل. **تم إنشاؤه** (`options ath9k_htc use_dev_fw=1`) — بعد reboot جديد تحمّل 1.5 تلقائياً ✓.
3. **mac80211 power-save يعيد ظل ريجسترات MAC في ROM** عند كل wake — أي knob ريجيستر يحتاج `iw dev ... set power_save off` (أو ترافيك) ليحتفظ بقيمته. السكربت يعزله.
4. **parser artifact**: أرقام "46K timeouts" التي ظهرت مؤقتاً كانت خطأ parsing في health() (الـ diag صار متعدد الأسطر) — لا timeouts حقيقية خلفية. أُصلح.
5. `probe-wmi-bisect.sh` و`probe-usbmon-capture.sh` و`probe-deploy-capture.sh`: أدوات العزل الحي الجديدة — الـ bisect يثبت A/B/C لكل عملية (ناجح/فاشل-والهدف-حي/القناة-ماتت).

## 5️⃣ ما زال مفتوحاً (بصدق)

| البند | الحالة |
|---|---|
| **ack_timeout من طرف إلى طرف** | مسار WMI ✓ + blackbox ✓ لكن 0x8014 لا يحتفظ بقيمة الـ host (mac80211/ROM يعيدون الكتابة). يحتاج قرار تصميم: إما تعطيل إعادة الكتابة في مسار host reset، أو جعل الـ knob يقرأ/يكتب عبر مسار fw خاص |
| **nf على راديو idle** | قيم حقيقية لكن غير معايرة بدون beacon sampling — مقبول توثيقياً؛ القيمة الحقيقية تظهر عند الارتباط |
| **رسائل timeout في dmesg** | تحتاج `-DCONFIG_ATH_DEBUG` في الكيرنل/الـ kbuild (اختياري) |
| **thermal avg_rssi** | يبقى 0 مع hooks=0 (العينات من RX hook المعطل بالعزل) — القيمة تعمل عند hooks=1 لكن عودتها معلقة على حسم استقرارها (docs/12) |
| **vicious wedge عند 64B** | آلية الـ jam داخل ROM/USB controller — تجنبناها بالحجم؛ لا إصلاح ممكن داخل الـ ROM |

## 6️⃣ أدوات الجلسة (في `ath9k-fw-build/`)

- `deploy.sh` (محدث: عزل NM + settle ping إجباري + فحوص صارمة + exit codes)
- `test-all.sh` (محدث: root-check، D مُتحقق، tee مع errno، health بين الأقسام، PASS/FAIL ملخص، عزل power_save + scan تهيئة، exit code صحيح)
- `probe-fresh-reload.sh` / `probe-wmi-bisect.sh` / `probe-usbmon-capture.sh` / `probe-deploy-capture.sh`
- `sudoers-ath9k-diag` — **احذف `/etc/sudoers.d/99-ath9k-diag` بعد انتهاء الحاجة للـ root-diag**

📅 آخر تحديث: 2026-11-15

---

## 7️⃣ تصحيح لاحق (2026-11-15 مساءً — docs/14)

- **قراءة NF "-43 dBm مرتبطة" في Fix 4 أعلاه لم تكن NF حقيقية** — كانت قراءة حقل خاطئ: `read_file_nf` في `htc_drv_debug.c` كان يقرأ حقل MINCCA بإزاحة 19 (تعريف AR5008) بينما AR9002/9271 يخزنه في البتات 28..20 (تعريف `AR9280_PHY_MINCCA_PWR`، مطابق لـ `ar9002_hw_do_getnf()` الرسمي). الإصلاح v4.1 مع طباعة `raw=` للتحقق — القيمة المرتبطة الصحيحة الآن **-116/-118 dBm** داخل نافذة الجودة الرسمية للـ 9271 (nominal -118، good -127..-116). كذلك كانت -236/-224 عند الخمول زوال مرجعية لنفس الحقل المزاح.
- **حادثة حية أثناء الـ soak**: تعليق حقيقي عند 17:22 (ping ميت + عدادات متجمدة) — الـ watchdog رصده (~15-20s) ونفّذ unbind/bind ×2 + usbreset ثم طبّق حد المعدل بشكل صحيح. النقص المكتشف: إعادة تأسيس DHCP (NM) لم تكن خطوة — أُضيفت في v4.2 (خطوة 2 قبل usbreset)، مع إعادة اكتشاف الـ gateway كل فحص (الشبكة غيّرت subnet أثناء الحدث).
