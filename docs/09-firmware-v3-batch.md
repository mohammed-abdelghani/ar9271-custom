# 🧠 دفعة v3 — 13 ميزة إقامة كاملة في الفيرموير

> **التاريخ:** 2026-11-14 | **الكيرنال:** 6.12.117 | **الفيرموير:** 51,140 بايت — **46 أمر WMI**
> المرحلة الثالثة: كل الميزات تتنفذ داخل معالج Xtensa، والدرايفَر واجهة لها فقط.
> **قاعدة الصنعة:** كل ردود WMI ≤ 64 بايت (سقف مجمع الرد — تجاوزه = assert داخل الشريحة، موثق في [08](08-firmware-resident-features.md)).

## 📋 الميزات الـ 13

| # | الميزة | واجهة debugfs | ينفذها |
|---|---|---|---|
| 1 | سلسلة الإرسال الكاملة runtime (tries/rates) | `rc_profile` (يقرا 4 قيم حية) | الطرفان |
| 2 | نافذة Block-ACK runtime (1-64، تُقرب لأقرب قوة 2) | `baw` | الفيرموير (يطبق فوراً على كل الـ TIDs الحية) |
| 3 | **Long-Distance Mode** — ACK timeout بالـ µs (سجل `0x8014`) | `ack_timeout` | الفيرموير |
| 4 | **Time-on-Air per Rate** — تقدير وقت الهواء لكل معدل شاملاً إعادة الإرسال | `rate_usage` (امتداد) | `ratectrl_11n_ln.c` + `if_owl.c` |
| 5 | **NF Survey لكل قناة** — الدرايفَر يبلغ بالتردد، الشريحة تربطه بـ NF | `chan_survey` | الطرفان |
| 6 | **مراقب البيكون** — عداد في كل نافذة (إنذار مبكر للضعف) | `telemetry` | `if_ath.c` |
| 7 | **Mini-NetFlow في الراديو** — 6 MACs × فريمات/بايت (تعبئة تلقائية) | `netflow` | `if_ath.c` (خطاف RX) |
| 8 | **Auto-ANI v2** — firstep + **إزاحة CCA threshold** ±3dB | `ani_auto` | `if_ath.c` |
| 9 | **Auto-Spectral** — مسح طيفي تلقائي عند انفجار أخطاء PHY | `auto_spectral` | الفيرموير (سجل `0x9910`) |
| 10 | **Black Box 📦** — ring buffer 16 حدث (ANI steps، watchdog، spectral، قنوات...) | `blackbox` (4 صفحات) | `if_ath.c` |
| 11 | عداد الفريمات المفلترة | `macfilter` + `telemetry` | `if_ath.c` |
| 12 | بصمة post-mortem عبر الـ black box (watchdog strikes قبل أي FATAL) | `blackbox` | `if_ath.c` |
| 13 | LED patterns | — | مؤجلة (تكلفة workqueue جديدة مقابل قيمة منخفضة — موثق) |

## 🔌 أوامر WMI الجديدة (v3)

| ID | الأمر | الاتجاه |
|---|---|---|
| 0x0028 | `WMI_CUSTOM_BAW_CMDID` | host→target |
| 0x0029 | `WMI_CUSTOM_LONGDIST_CMDID` | host→target |
| 0x002A | `WMI_CUSTOM_FREQ_NOTE_CMDID` | host→target (بـ timeout قصير 100ms، يُرسل عند كل تغيير قناة من الدرايفَر) |
| 0x002B | `WMI_CUSTOM_NETFLOW_CMDID` | host→target |
| 0x002C | `WMI_CUSTOM_CHAN_SURVEY_CMDID` | host→target |
| 0x002D | `WMI_CUSTOM_AUTO_SPEC_CMDID` | host→target |
| 0x002E | `WMI_CUSTOM_BLACKBOX_CMDID` | host→target |

**الإجمالي: 46 أمر WMI** (31 رسمي + 15 مخصص) + event معكوس واحد.

## 🧪 الاختبار (بعد deploy)

```bash
sudo /home/mohamed/Projects/ath9k-fw-build/deploy.sh
sudo /home/mohamed/Projects/ath9k-fw-build/test-all.sh
```

## ⚙️ تفاصيل هندسية

- **airtime**: يحسب عند بناء السلسلة `frameLen×8×1000/rateKbps` لكل خانة (يشمل إعادة الإرسال) — تقدير إشغال القناة لكل معدل
- **BAW**: تُطبق فوراً على كل الـ TIDs الحية (`ATH_AN_2_TID` على `sc_sta[]` الصالحة) وعلى الجديدة
- **NF survey**: الفيرموير لا يتتبع القناة (النواة تبرمجها بسجلات مباشرة) — لذلك الدرايفَر يبلغها بأمر صغير، والشريحة تقرا الـ NF latch وتخزن الزوج (تردد↔ضجيج). الجدول 12 خانة، والامتلاء يستبدل الأسوأ NF
- **auto-spectral**: يكتب `AR_PHY_SPECTRAL_SCAN (0x9910) |= ENA|ACTIVE` مباشرة، ويعطل بعد 3 نوافذ تلقائياً
- **الذاكرة**: الإضافات ~350 بايت statics — headroom كان 4.9KB

📅 آخر تحديث: 2026-11-14
