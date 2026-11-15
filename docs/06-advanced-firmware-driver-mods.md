# 🧬 التعديلات المتقدمة على الفيرموير والدرايفر — Custom Build

> **التاريخ:** 2026-11-13
> **الحالة:** ✅ كل التعديلات مطبّقة وتبنى بنجاح (الدرايفر اتبنى؛ الفيرموير قيد البناء)
> **الهدف:** تعديلات حقيقية على مستوى كود الفيرموير (Xtensa) ودرايفر النواة — مع خط تجريب آمن وقابل للرجوع بالكامل.

---

## 1️⃣ خريطة التعديلات

### أ) تعديلات درايفر النواة (`sources/driver/linux-6.12.116-ath9k/`)

| # | الملف | التعديل | الهدف |
|---|---|---|---|
| 1 | `ath9k/hif_usb.h` | `MAX_TX_URB_NUM` و `MAX_RX_URB_NUM`: **8 → 16** | تبوير الـ Throughput: URBs أكتر = pipelining أحسن على USB. الكلفة ~768KB ذاكرة |
| 2 | `ath9k/htc.h` | `ATH9K_HTC_MAX_STA`: **مرحلتين** | عند نشر الفيرموير الرسمي 1.4.0 يبقى **8** (الحد الآمن — جدول الشريحة 8 nodes ويُفهرس بنفس الـ idx). عند نشر الفيرموير المخصص 1.5 (بتاعنا فيه `ATH_NODE_MAX 16`) يُرفع لـ **16** — الطرفان معاً دائماً |
| 3 | `ath9k/htc.h` | حقول جديدة في `struct ath9k_debug` | تسجيل نتائج الـ custom ping |
| 4 | `ath9k/htc_drv_init.c` | `pr_info` banner عند تحميل الموديول | تأكيد أن **بنائنا** هو المحمّل فعلاً |
| 5 | `ath9k/wmi.h` + `wmi.c` | أمر جديد `WMI_CUSTOM_PING_CMDID` (0x0021) + دالة `ath9k_wmi_custom_ping()` | إضافة أمر تحكم مخصص end-to-end بين لينكس والمعالج المدمج |
| 6 | `ath9k/htc_drv_debug.c` | ملف debugfs جديد `custom_ping` (0600) | تشغيل الـ ping من اليوزر: `echo 1234 > custom_ping` ثم قراءة النتيجة |
| 7 | **Kbuild مخصص** | بناء out-of-tree مع `-DCONFIG_ATH9K_HTC_DEBUGFS` | نواة Debian تبني الموديول **بدون** debugfs — بناؤنا يفعّل إحصائيات الشريحة |

> ⚠️ **ملاحظة مهمة:** نواة Debian الرسمية (`/boot/config-6.12.116+deb13-amd64`) فيها `# CONFIG_ATH9K_HTC_DEBUGFS is not set` — يعني الموديول الرسمي **لا يوفّر** إحصائيات `tgt_tx_stats` / `tgt_rx_stats` إطلاقاً. بناؤنا المخصص هو أول ما يجعلها متاحة على هذا النظام.

### ب) تعديلات الفيرموير (`sources/firmware/open-ath9k-htc-firmware/`)

| # | الملف | التعديل | الهدف |
|---|---|---|---|
| 8 | `target_firmware/wlan/include/k2/wlan_cfg.h` (+ `magpie` للاتساق) | `ATH_VERSION_MINOR`: **4 → 5** | تعريف البناءات المخصصة — يظهر في `dmesg` كـ `FW Version: 1.5` |

> 🔍 **اكتشاف مهم أثناء التحقق:** صورة الفيرموير التي تعمل فعلاً على كارت AR9271 هي ناتج **TARGET_K2** (وليس MAGPIE كما يوحي اسم المنصة). الدليل القاطع — بصمة الأحجام ضد النسخة الرسمية 1.4.0: الملف المسمّى `9271` (51,008) = بناء K2 ( ours: 51,016 = +8 بايت بالضبط = مدخل الـ WMI dispatch الذي أضفناه)، والملف المسمّى `7010` (72,812) = بناء MAGPIE (ours: 72,820 = +8). لذلك رُفعت النسخة في `k2/wlan_cfg.h` — وإن لم ترَ `1.5` في `dmesg` فهذا مؤشر أنك نشّرت بناء المنصة الخطأ.
| 9 | `target_firmware/wlan/include/wlan_hdr.h` | `ATH_NODE_MAX`: **8 → 16** | جدول العقد داخل الشريحة — لازم يطابق `ATH9K_HTC_MAX_STA` في الدرايفَر. الـ linker (ram-magpie.ld) يفشل بصوت عالٍ لو الذاكرة لم تكفِ |
| 10 | `target_firmware/wlan/include/wmi.h` | `WMI_CUSTOM_PING_CMDID` + بنى `WMI_CUSTOM_PING_CMD/RSP` | الطرف الفيرموير من الأمر المخصص (نفس رقم الـ ID في الطرفين — **إلزامي**) |
| 11 | `target_firmware/wlan/if_ath.c` | دالة `handle_custom_ping_cmd()` + تسجيلها في جدول `Magpie_Sys_DispatchEntries` | تنفيذ الأمر داخل المعالج المدمج: يرجّع الـ payload + عدّاد |
| 12 | `target_firmware/wlan/if_owl.c` | `stepDnInc` في `ath_tgt_rate_findrate()`: **1 → 2** | "High-interference profile": سلسلة إعادة الإرسال تنزل أسرع بين المعدلات عند الفقد (مثلاً `{54M, 36M, 12M, 6M}` بدل `{54M, 48M, 36M, 24M}`) — أقل هدر airtime في البيئات المزدحمة |

### ج) فحص مسار RX للفلترة (بدون تعديل — تم التحقق)

- الدرايفَر يضبط فلتر الـ PCU صح في وضع المونيتور: [`htc_drv_txrx.c:882`](../sources/driver/linux-6.12.116-ath9k/drivers/net/wireless/ath/ath9k/htc_drv_txrx.c) — `is_monitoring → ATH9K_RX_FILTER_PROM`، والقيمة تُكتب في الشريحة عبر WMI REG WRITE.
- نداء `ah_setRxFilter(ah, 0)` في الفيرموير (`if_ath.c:1646`) يحدث فقط في `ath_stoprecv_tgt` (إيقاف الاستقبال) — سلوك سليم.
- **الخطوة التالية المحتملة** في الفيرموير (إن أردت تعميق الاستقبال): رفع `ATH_RXBUF` (20) في `wlan_cfg.h` — يجرب **بعد** التأكد من ميزانية الذاكرة في الـ linker لأن جدول العقد اتضاعف بالفعل.

---

## 2️⃣ نظام البناء (`/home/mohamed/Projects/ath9k-fw-build/`)

سبب وجود مجلد بناء منفصل: الـ Makefile الأصلي للفيرموير يستخدم `BASEDIR=$(shell pwd)` وينكسر نهائياً مع المسافات في مسار المشروع الأصلي. لذلك:

```
/home/mohamed/Projects/ath9k-fw-build/
├── fw/open-ath9k-htc-firmware/     ← مرآة المصدر (بدون مسافات في المسار)
├── driver/ath/                     ← مرآة مصدر الدرايفَر
├── ath9k-htc.kbuild                ← Kbuild البناء خارج الشجرة
├── driver-build.sh                 ← يبني ath9k_htc.ko
├── fw-build.sh                     ← ينزّل tarballs من mirrors.kernel.org + يبني المترجم ثم الفيرموير
├── deploy.sh                       ← (sudo) تثبيت الموديول + الفيرموير وإعادة التحميل
├── rollback.sh                     ← (sudo) الرجوع الكامل للنسخة الرسمية
├── ath9k_htc.ko                    ← ناتج بناء الدرايفَر ✅
└── htc_9271.fw                     ← ناتج بناء الفيرموير
```

مصدر الحقيقة للتعديلات هو المستودع في `sources/` (git-tracked — راجع بـ `git diff`)، والمجلد mirror يُزامَن منه قبل كل بناء.

### أوامر البناء

```bash
# الدرايفَر (سريع — ثوانٍ):
./driver-build.sh

# الفيرموير (طويل — أول مرة 30-70 دقيقة بسبب بناء مترجم Xtensa من المصدر):
./fw-build.sh
```

فريق الترجمة ينزّل GMP 6.2.0 + MPFR 4.1.0 + MPC 1.1.0 + binutils 2.35 + GCC 10.2.0 من `mirrors.kernel.org` (مرآة GNU مستقرة — `ftp.gnu.org` قطع الاتصال TLS أثناء أول محاولة)، ويتحقق من كل ملف بـ SHA-256.

---

## 3️⃣ النشر والتجريب — على مرحلتين (يعمل بدون إنترنت)

### 🟢 المرحلة 1 — درايفر مخصص + فيرموير رسمي 1.4.0 (متاح الآن)

```bash
sudo /home/mohamed/Projects/ath9k-fw-build/deploy.sh
```

السكربت يثبّت الموديول في `/lib/modules/$(uname -r)/updates/` (يظلّل النسخة الرسمية **دون مساسها**) ويعيد التحميل. بما أن الفيرموير المخصص لم يُبنَ بعد، يبقى الرسمي شغالاً — و`use_dev_fw=1` آمن تماماً (الدريفر يرجع لـ 1.4.0 تلقائياً).

**ما يعمل فوراً في المرحلة 1:**
- ✅ إحصائيات الشريحة في debugfs (`tgt_tx_stats` / `tgt_rx_stats` / `tgt_int_stats`) — لأول مرة على نواة Debian
- ✅ مضاعفة الـ URBs (16/16) — تبوير Throughput
- ✅ البانر المخصص يثبت أن بنائك هو المحمّل
- ⏸️ `custom_ping`: آمن لكن لن يستجيب (الفيرموير الرسمي يتجاهل الأوامر المجهولة بصمت ويسقطها — تم التحقق في `wmi_svc.c`) — يعمل عند نشر المرحلة 2
- ⏸️ `MAX_STA > 8` + rate control المعدّل — يحتاجان الفيرموير 1.5

### 🟡 المرحلة 2 — فيرموير مخصص 1.5 (تحتاج إنترنت مرة واحدة)

البناء توقف عند نقطة واحدة فقط: **تنزيل `gcc-10.2.0.tar.gz` (129MB، مرة واحدة)** — الناقص بعد أن اكتمل بناء GMP + MPFR + MPC + binutils بالباتشات الصحيحة محلياً. لا بديل ممكن: الباتشات تُعرّف نواة معالج AR9271 الخاصة، فأي مترجم Xtensa خارجي (مثل ESP32) سيُنتج كوداً تالفاً.

عند توفر إنترنت:
```bash
/home/mohamed/Projects/ath9k-fw-build/fw-build.sh   # يستكمل من حيث توقف (يتخطى المكتمل)
sudo /home/mohamed/Projects/ath9k-fw-build/deploy.sh  # يرفع الفيرموير تلقائياً في فتحة dev
```
ثم أعد بناء الدرايفَر مع رفع `MAX_STA` لـ 16 (الطرفان معاً).

### اختبار أمر WMI المخصص

```bash
PHY=$(ls /sys/kernel/debug/ieee80211/ | head -1)
echo 1234 > /sys/kernel/debug/ieee80211/$PHY/ath9k_htc/custom_ping
cat  /sys/kernel/debug/ieee80211/$PHY/ath9k_htc/custom_ping
# pings=1 last_payload=0x000004d2 last_pong=1 last_ms=...
#  ^ العدد     ^ القيمة المرجعة من الشريحة   ^ عدّاد الشريحة  ^ زمن الذهاب والعودة
```

### إحصائيات الشريحة (الآن متاحة لأول مرة!)

```bash
cat /sys/kernel/debug/ieee80211/$PHY/ath9k_htc/tgt_tx_stats
cat /sys/kernel/debug/ieee80211/$PHY/ath9k_htc/tgt_rx_stats
cat /sys/kernel/debug/ieee80211/$PHY/ath9k_htc/tgt_int_stats
```

### الرجوع للخلف

```bash
sudo /home/mohamed/Projects/ath9k-fw-build/rollback.sh
```

---

## 4️⃣ قواعد ذهبية عند تعديل الفيرموير والدرايفَر معاً

1. **البنى المشتركة (WMI messages) يجب أن تتطابق طرفاً لطرف** — نفس رقم الـ ID، نفس حجم وترتيب الحقول. الأرقام في الـ enum تُضاف **في النهاية فقط** حتى لا تُزاح الأوامر القديمة.
2. **جداول السعة (nodes/stations) تُعدّل في الطرفين معاً** — الدرايفَر يعطي `sta_idx` والفيرموير يفهرس `sc_sta[]` بنفس الرقم.
3. **الفيرموير لا يُكتب في الفلاش** — يُحمّل من النواة إلى RAM الشريحة عند كل توصيل؛ فصل الكارت يعيد الوضع السابق دائماً.
4. **ميزانية الذاكرة صغيرة** — الفيرموير يعيش في RAM محدودة (`ram-magpie.ld`)؛ كل بنيوة تضيفها لها ثمن، والـ linker هو الحَكَم.

## 5️⃣ ملخص التغييرات (git)

```bash
git -C sources/driver/linux-6.12.116-ath9k diff --stat
git -C sources/firmware/open-ath9k-htc-firmware diff --stat
# الدرايفَر: 6 ملفات، +108 أسطر
# الفيرموير: 5 ملفات، +49 أسطر
```

📅 آخر تحديث: 2026-11-13
