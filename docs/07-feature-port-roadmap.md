# 🚀 خارطة نقل المميزات من كروت أخرى — WN422G v2 Custom Build

> **التاريخ:** 2026-11-13
> **الأساس:** مصنع فيرموير + درايفَر مخصص شغال (راجع [06](06-advanced-firmware-driver-mods.md)) — أي ميزة جديدة بتتجمع في دقائق.
> **النطاق:** كل المميزات الممكنة من كروت أخرى، ما عدا المستحيلة هاردويرياً (5GHz، MIMO 2x2، 11ac/ax، BT coex، WoWL).

## 📊 الحالة النهائية (مُنفذة ومُختبرة)

| # | الميزة | الحالة | واجهة المستخدم |
|---|---|---|---|
| 1 | 802.11n HT20/HT40 + MCS 0-7 | ✅ مُتحقق حياً (`iw list`) | تلقائي عند الاتصال |
| 2 | Spectral Scan (محلل طيف FFT) | ✅ مُتحقق حياً (فريمات 64B × 56 bin) | `spectral_scan0` + `spectral_scan_ctl` |
| 3 | Rate Control runtime profiles | ✅ مُتحقق حياً (echo 1→1, 2→2) | `rc_profile` |
| 4 | ANI-lite firstep tuner | ✅ مبني | `firstep` (0-2) |
| 5 | TX Power override لكل فريم | ✅ مُتحقق حياً (60→40→63) | `txpower` |
| 6 | Fixed-rate runtime toggle | ✅ مُتحقق حياً (5↔255) | `rc_fixed` |
| 7 | Noise Floor حي + CCA knob | ✅ مُتحقق حياً (قراءة بالديسيبل) | `nf` |

**أوامر WMI المخصصة في الفيرموير (v1.5):** `WMI_CUSTOM_PING_CMDID` (0x21)، `WMI_CUSTOM_RC_PROFILE_CMDID` (0x22)، `WMI_CUSTOM_TXPOWER_CMDID` (0x23) — كلها بنمط اكتشاف/تطبيق/رد تأكيد من الشريحة.

**ملاحظات قياس:**
- `noise_floor` يتحدث مع نشاط الكارت — والقيمة الأدق تُقرأ أثناء الاتصال/المسح (عند الخمول قد تكون قيمة قديمة مخزنة)
- `txpower` (0-63) هو فهرس PDACC في جدول القدرة المعاير للقناة — 60 هو الافتراضي المصنعي للفريمات
- `rc_fixed` يستخدم فهرس جدول معدلات 11ng (0-11 ليجاسي CCK/OFDM، 12-19 = MCS0-7)

---

## 📋 الخطوات بالترتيب

### الخطوة 1 — ✅ تحقق 802.11n الحي — **مُتحقق ويعمل (2026-11-13)** ⭐

**نتيجة الفحص الحي (`iw list` على phy الكارت):**
```
Capabilities: 0x116e
    HT20/HT40
    HT TX/RX MCS rate indexes supported: 0-7
```
الكارت بيعلن رسمياً HT20+HT40 مع MCS 0-7 — تسمية "802.11g 54Mbps" تسويقية بالكامل، والتفاوض مع راوترات 11n شغال تلقائياً بلا أي تعديل. (ووضعيات الواجهة المتاحة: IBSS، managed، AP، AP/VLAN، monitor، mesh point، P2P-client/GO).

**الوضع الحالي (متحقق من السورس):** السلسلة كاملة —
- `hw.c:2574`: `CAP_HT` مسجل لكل الشرائح
- `htc_drv_main.c:24`: الدرايفَر يبعث `HTC_MODE_11NG` **دايماً** على 2.4GHz
- `htc_drv_main.c:613-615`: تفاوض MCS لكل شبكة عبر WMI
- الفيرموير: `ratectrl_11n_ln.c:675` وضع `IEEE80211_MODE_11NG` كامل

**خطوات التحقق:**
```bash
sudo apt install iw
iw phy   # البحث عن "HT20" في قدرات الكارت
# الاتصال براوتر 11n ثم:
iw dev wlx001122334455 link   # فحص المعدل المفاوض (MCS أو ليجاسي)
```
- لو MCS ظهر → الميزة شغالة أصلاً والتسمية 54Mbps تسويقية
- لو انحشرت على legacy → نحدد الـ gate (EEPROM opflags أو ht_cap masking) ونرقعه في بنائنا

### الخطوة 2 — 🌟 تفعيل Spectral Scan / محلل الطيف — **✅ شغال ومُتحقق حياً (2026-11-13)** ⭐⭐⭐

**ما تم:** بناء `ath9k_common.ko` بالـ `CONFIG_ATH9K_COMMON_SPECTRAL` + نشره في `updates/` — ستة ملفات spectral ظهرت في debugfs (`spectral_scan_ctl`, `spectral_scan0`, `spectral_period`, `spectral_fft_period`, `spectral_count`, `spectral_short_repeat`).

**الأوامر الصحيحة** (القيم المقبولة في `spectral_scan_ctl`: `manual` / `trigger` / `background` / `chanscan` / `disable` — لا يوجد "enable"):
```bash
N=$(cat /sys/class/net/wlx001122334455/phy80211/index)
D=/sys/kernel/debug/ieee80211/phy$N/ath9k_htc
echo manual  | sudo tee $D/spectral_scan_ctl
echo trigger | sudo tee $D/spectral_scan_ctl
sudo ip link set wlx001122334455 down; sudo iw dev wlx001122334455 set type monitor; sudo ip link set wlx001122334455 up
sudo iw dev wlx001122334455 set channel 6
sudo timeout 10 cat $D/spectral_scan0 > /tmp/spec0.bin
echo disable | sudo tee $D/spectral_scan_ctl
```

**الدليل الحي:** 608 بايت في 10 ثواني — فريمات `fft_sample_ht20` بسلسلة 64 بايت (8 رأس + 56 bin) بتتكرر بدقة، والـ bins فيها مقاييس طاقة حية لقناة 6. الكارت بقى محلل طيف RF فعلي.

**المشكلة:** `ath9k_common` بتاع Debian مبني بدون `CONFIG_ATH9K_COMMON_SPECTRAL` → كود الـ spectral في `htc_drv_debug.c:545` بيتحول لـ stubs → مفيش `spectral_scan_ctl` في debugfs.

**خطوات التنفيذ:**
1. توسيع `ath9k-htc.kbuild` لبناء `ath9k_common.ko` كمان بالخانة مفعلة:
   `ath9k_common-y := common.o common-init.o common-beacon.o common-debug.o common-spectral.o`
2. فحص gating الشريحة في `common-spectral.c` (فحوصات SREV) — تعديل إن لزم لـ AR9271
3. النشر في `updates/` (يظلل نسخة الـ distro — مع rollback)
4. اختبار FFT: `spectral_scan_ctl` → enable → قراءة الباكتيتس بـ Wireshark (radiotap DSP header)

**الناتج:** محلل طيف حي يرسم توزيع الطاقة في 2.4GHz — أداة كشف تداخل حقيقية.

### الخطوة 3 — 🎛️ Rate Control runtime profiles عبر WMI ⭐⭐⭐⭐

نفس نمط الـ `custom_ping` المثبت:
1. **فيرموير:** `WMI_RC_PROFILE_CMDID` جديد — handler يخزن `stepDnInc`/`numTries` في متغيرات runtime تقرأها `ath_tgt_rate_findrate()` بدل الثوابت
2. **درايفَر:** نفس الـ ID في `wmi.h` + دالة + ملف debugfs `rc_profile`
3. بروفايلان جاهزان: `clean` (stepDnInc=1) و `noisy` (stepDnInc=2) — تبديل فوري بدون rebuild

### الخطوة 4 — 🎚️ ANI tuning عبر WMI (من كروت PCI ath9k) ⭐⭐⭐⭐

1. فحص تطبيق ANI داخل الفيرموير (`ar5416Phy.c` — noise/spur immunity)
2. أوامر WMI جديدة لضبط المعاملات + debugfs في الدرايفَر
3. النتيجة: تحكم حي في حساسية الاستقبال مقابل مقاومة التداخل (كما في debugfs كروت PCI)

### الخطوة 5 — ⚡ TX Power override عبر WMI ⭐⭐⭐

1. Handler في الفيرموير يعدّل حد القدرة فوق/تحت قيم الـ EEPROM (ضمن الحدود القانونية)
2. debugfs `txpower_override` في الدرايفَر
3. ملاحظة: يبقى للأبحاث داخل نطاق الترخيص المحلي

### الخطوة 6 — 🔒 Fixed-rate runtime toggle ⭐⭐⭐

- `ATH_DISABLE_RC` موجود كثابت compile في `wlan_cfg.h` ("Use fixed rate instead of rate control")
- يتحول لمتغير runtime يُضبط عبر WMI → اختبار معدلات ثابتة بدون إعادة بناء (مفيد للمقارنات والـ benchmarks)

### الخطوة 7 — 📡 CCA/NAV threshold tuning (من كروت outdoor) ⭐⭐⭐⭐

1. تحديد سجلات حساسية CCA في PHY الـ AR9271 (من `ar5416reg.h`)
2. debugfs `cca_threshold` ينفذ كتابة السجلات عبر `WMI_REG_WRITE` الموجود أصلاً
3. مفيد للروابط البعيدة (قبول ضوضاء أعلى) — بتجربة دقيقة

---

## 🧭 ترتيب التنفيذ المقترح

| الترتيب | الخطوة | السبب |
|---|---|---|
| 1 | الخطوة 1 (تحقق 11n) | ⭐ بدون بناء — إثبات حي فوري |
| 2 | الخطوة 2 (Spectral) | أكبر مكسب — درايفَر فقط |
| 3 | الخطوة 3 (RC profiles) | يبني على نمط WMI المثبت |
| 4-7 | الباقي | نفس النمط التصاعدي |

## ⚠️ قواعد ثابتة لكل خطوة

1. أوامر WMI الجديدة تُضاف **في نهاية الـ enum** في الطرفين معاً
2. أي struct مشترك: نفس الحجم والترتيب — الجهاز مش هيتبوظ (فصل وإرجاع)
3. النشر في `updates/` (درايفَر) و `htc_9271-1.dev.0.fw` (فيرموير) — الأصلي لا يُمس
4. `rollback.sh` يرجع كل شيء

📅 آخر تحديث: 2026-11-13
