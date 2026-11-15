# 📡 Custom ath9k_htc Driver & Firmware — TP-Link TL-WN422G v2 (Atheros AR9271)

![Hardware](https://img.shields.io/badge/hardware-AR9271_%22Magpie%2FK2%22-orange)
![Driver](https://img.shields.io/badge/driver-ath9k__htc_v4.1-blue)
![Firmware](https://img.shields.io/badge/firmware-custom_v1.5_51_144B-8A2BE2)
![WMI](https://img.shields.io/badge/WMI-46_commands_(15_custom)-success)
![Tested](https://img.shields.io/badge/test--suite-51_PASS%20%2F%202_documented-critical)
![Platform](https://img.shields.io/badge/platform-Debian_Linux_6.12-informational)
![License](https://img.shields.io/badge/license-GPL--2.0-green)

> A complete, measured, root-caused engineering effort to turn a cheap "54 Mbps" USB Wi-Fi stick into a
> fully instrumented research radio: **custom Xtensa firmware** (13 firmware-resident features, 15 custom WMI
> commands, a 64-byte-packet USB wedge found and fixed), a **custom driver build** (live debugfs knobs, honest
> noise-floor reads), and **host-side resilience** (a hang watchdog that earned its keep in a real hang event).
> Everything in this repo is backed by evidence: usbmon captures, register cross-decodes, and a 2× stability soak.

**العربية:** ⬇️ القسم العربي في [أسفل الصفحة](#-القسم-العربي).

---

## ✨ Highlights

| Area | What you get | Evidence |
|---|---|---|
| 🔬 **Firmware-resident features** | spectral-triggered auto-scan, thermal governor, auto-ANI v2 (firstep + CCA), RSSI histogram, radio-side MAC filter, mini-netflow, channel NF survey, Block-ACK window, rate-control runtime profiles, TX-power override, reverse telemetry events, post-mortem black box | [08](docs/08-firmware-resident-features.md), [09](docs/09-firmware-v3-batch.md), [12](docs/12-thermal-batch.md) |
| 🎛️ **Live debugfs control** | ~25 runtime knobs/files on the stock driver path — no rebuild to tune | [docs/](docs/README.md) |
| 🎯 **Root-caused mystery** | "dies after ~2 custom replies" = any WMI control packet of **exactly 64 B on the wire** wedges the USB controller; proven with usbmon (`EOVERFLOW` × 1401), fixed by shrinking payloads + capping the credit trailer | [13](docs/13-wmi-wedge-root-cause.md) |
| 📏 **Honest measurements** | noise-floor decode bug found (wrong register field, shift 19 vs 20) — NF now reads **-116/-118 dBm** inside the vendor quality window, with raw-register cross-decode proof | [14](docs/14-stability-pass-final-20261115.md) |
| 🛡️ **Self-healing host** | watchdog with detection → unbind/bind → NM-reconnect → usbreset ladder, rate-limited, gateway re-derived per check; validated during a **real hang** (channel-hop + subnet-change event) | [14 §2a](docs/14-stability-pass-final-20261115.md) |
| 📐 **Chip architecture docs** | Mermaid diagrams (EN + AR): SoC blocks, USB endpoint map, TX/RX paths, WMI round-trip, firmware memory map | [15 EN](docs/15-chip-diagrams-en.md) · [16 AR](docs/16-chip-diagrams-ar.md) |

## 📐 Architecture at a glance

```mermaid
flowchart LR
    subgraph H["Host — Linux 6.12"]
        NM["NetworkManager"] --- M802["mac80211"] --- DRV["ath9k_htc v4.1<br/>~25 debugfs knobs"]
    end
    subgraph C["AR9271 — Xtensa firmware v1.5"]
        WMI2["46 WMI commands<br/>15 custom (0x21-0x2F)"]
        FEAT["thermal governor · auto-ANI<br/>macfilter · netflow · blackbox …"]
    end
    ANT["2.4 GHz RF<br/>HT20/HT40 MCS 0-7"]
    DRV -- "EP4 OUT: cmds" --> WMI2
    WMI2 -- "EP3 IN: replies &lt; 64 B" --> DRV
    WMI2 --- FEAT --- ANT
```

## 📦 Repository structure

```text
├── README.md                     ← you are here (EN + AR)
├── docs/                         ← the full documentation set (see docs/README.md for the index)
│   ├── 01…04   hardware · stock driver/firmware · device status · usage
│   ├── 06…09   custom build pipeline · feature roadmap · firmware batches v2/v3
│   ├── 10…14   debug log · host tuning · thermal batch · WMI-wedge root cause · final stability pass
│   ├── 15…16   AR9271 chip diagrams (EN / AR, Mermaid)
│   └── test-final-20261115.log   final test-suite evidence
├── patches/                      ← the delta vs upstream, as plain diffs (for applying to pristine trees)
│   ├── ath9k-driver-v6.12.116-custom.patch        driver mods incl. v4.1 (ATH_DEBUG + NF mask fix)
│   └── open-ath9k-htc-firmware-custom.patch       firmware mods: 13 features + 64B-wedge fixes
├── sources/                      ← the ALREADY-MODIFIED, working source trees (see sources/README.md)
│   ├── linux-6.12.116-ath9k/             kernel v6.12.116 drivers/net/wireless/ath/ subtree — 7 modified files
│   └── open-ath9k-htc-firmware/          upstream firmware master — 12 modified files
├── firmware/
│   └── htc_9271-1.dev.0.fw       final firmware binary (51,144 B) — flashable without the Xtensa toolchain
└── scripts/
    ├── build/                    driver-build.sh · fw-build.sh · ath9k-htc.kbuild
    ├── deploy.sh · test-all.sh · rollback.sh · reset-target.sh
    ├── apply-stability-tuning.sh · soak-2h-v2.sh
    ├── ath9k-watchdog.sh · boot-fw-probe.sh
    ├── probes/                   live-isolation tools: wmi-bisect · usbmon-capture · reply-capacity …
    └── udev/91-ath9k-htc-stability.rules
```

## 🚀 Quick start

> ℹ️ The trees under `sources/` are **already patched and proven on hardware** — you don't need to apply
> anything. `patches/` records the same delta as plain diffs, if you prefer to start from pristine upstream
> (instructions in [sources/README.md](sources/README.md)). Build outputs go to a space-free mirror
> (`BUILD_DIR` env override, default `~/ath9k-fw-build`) because the firmware Makefile breaks on paths
> containing spaces.

**Prerequisites:** Debian-class Linux with kernel headers for your running kernel
(`linux-headers-$(uname -r)`, 6.12.x tested); `rsync`, `curl`, `bc`, `iw`, `usbutils`.

```bash
# 1) build (reads sources/ inside this repo)
./scripts/build/driver-build.sh      # → ath9k_htc.ko + ath9k_common.ko (spectral enabled)
./scripts/build/fw-build.sh          # → builds the Xtensa toolchain, then htc_9271.fw
                                     #   (first run ~15 min; or skip: flash firmware/htc_9271-1.dev.0.fw)

# 2) deploy + verify (root; installs into /lib/modules/.../updates/ + the dev fw slot)
sudo ./scripts/deploy.sh
sudo ./scripts/test-all.sh           # expect: 51 PASS / 2 documented FAILs (ack_timeout readback, idle NF)

# 3) host-side resilience (udev rule, NM powersave off, regdom, boot probe, watchdog service)
sudo ./scripts/apply-stability-tuning.sh
```

**Try the firmware features** (replace `$D` with your `phy*` path — see docs/06):

```bash
D=/sys/kernel/debug/ieee80211/phy$(cat /sys/class/net/wl*/phy80211/index)/ath9k_htc
sudo cat $D/thermal            # thermal governor status
sudo cat $D/blackbox           # last 16 in-chip events
sudo cat $D/custom_ping        # WMI round-trip sanity
```

## 🔑 Key findings (the parts you won't find upstream)

1. **The 64-byte WMI wedge** — a WMI control packet of *exactly* 64 B total on EP3 reaches the host successfully,
   then the chip's USB controller stops ACKing EP4 and floods `EOVERFLOW` every ~2 ms. Rule: keep the
   **total** (HTC + WMI + payload + credit trailer) **< 64 B**; the firmware now caps the trailer at 60 B.
2. **The noise-floor decode bug** — the 9271 stores `MINCCA_PWR` in bits 28..20 (`AR9280_PHY_MINCCA_PWR`), not
   bits 25..19 as the generic AR5008 mask assumes; all previously published NF values were wrong-field decodes.
3. **mac80211 + ROM shadowing** — MAC registers (e.g. ACK timeout `0x8014`) are re-written from ROM shadow on
   every wake, so per-knob persistence needs power-save off at minimum, and some knobs simply cannot hold.
4. **Memory budget discipline** — every firmware feature is measured against the ~3.7 KB IRAM margin
   (`.text` 35,467 B of 39,744 B); feature design starts from the byte budget.

## ⚠️ Responsible use

The TX-power knob (`txpower`) exists for measurement and link-budget research. Stay within your local
regulatory limits — the docs deliberately treat power above the calibrated defaults as out of scope.

---

# 🇦🇷 القسم العربي

# درايفر وفيرموير مخصصان — TP-Link TL-WN422G v2 (Atheros AR9271)

> جهد هندسي كامل ومقاس ومحسوم السبب الجذري لتحويل "كارت 54 Mbps" الرخيص إلى راديو بحثي مُجهّز بالكامل:
> **فيرموير Xtensa مخصص** (13 ميزة مقيمة في الشريحة، 15 أمر WMI مخصص، ولغز تعليق USB لحزم 64 بايت اتكشف وأتصلح)،
> **بناء درايفر مخصص** (حوالي 25 knob حي في debugfs، وقراءات noise floor صادقة)، و**حماية على الجهاز المضيف**
> (حارس تعليق أثبت نفسه في عطل حقيقي). كل حاجة هنا مسنودة بدليل: تصوير usbmon، وتفكيك ريجسترات، وsoak استقرار.

## 📚 الوثائق

| الملف | المحتوى |
|---|---|
| [docs/README.md](docs/README.md) | 📂 فهرس كامل للوثائق مع ترتيب قراءة مقترح |
| [docs/16-chip-diagrams-ar.md](docs/16-chip-diagrams-ar.md) | 🧩 **مخططات الشريحة بالعربية** — كتل الـ SoC، خريطة نقاط نهاية USB، مسارات الإرسال/الاستقبال، رحلة أمر WMI، خريطة الذاكرة |
| [docs/15-chip-diagrams-en.md](docs/15-chip-diagrams-en.md) | 🧩 نفس المخططات بالإنجليزية |
| [docs/06](docs/06-advanced-firmware-driver-mods.md) → [09](docs/09-firmware-v3-batch.md) | بناء مخصص من السورس + دفعات الميزات (v1 → v2 → v3) |
| [docs/13](docs/13-wmi-wedge-root-cause.md) | 🎯 السبب الجذري المحسوم بالدليل: حزم WMI بطول 64 بايت-تماماً تعلق متحكم USB |
| [docs/14](docs/14-stability-pass-final-20261115.md) | 📋 جولة الاستقرار النهائية: جدول تغييرات، قياسات، تعليمات تراجع دقيقة |

## 🚀 البداية السريعة

المتطلبات: لينكس مع هيدرات النواة (`linux-headers-$(uname -r)`، اختُبرت 6.12.x). شجرة `sources/`
**معدَّلة بالفعل ومثبتة على العتاد** — مش محتاج تطبّق أي patch؛ والـ `patches/` موجودة كسجل الفرق
لو عايز تبدأ من شجرة upstream نضيفة (التعليمات في [sources/README.md](sources/README.md)).
مخرجات البناء بتروح mirror فضاؤه خالي (`BUILD_DIR` env، الافتراضي `~/ath9k-fw-build`) لأن Makefile
الفيرموير يبوظ مع المسارات اللي فيها مسافات.

```bash
./scripts/build/driver-build.sh      # → ath9k_htc.ko + ath9k_common.ko (spectral مفعّل)
./scripts/build/fw-build.sh          # يبني toolchain Xtensa + الفيرموير (أول مرة ~15 دقيقة)
sudo ./scripts/deploy.sh && sudo ./scripts/test-all.sh     # المتوقع: 51 PASS / 2 FAIL موثقين
sudo ./scripts/apply-stability-tuning.sh                   # udev + powersave + regdom + watchdog
```

جرّب الميزات: `sudo cat $D/blackbox` و `sudo cat $D/thermal` و `sudo cat $D/custom_ping` —
والتفاصيل خطوة بخطوة في [docs/06](docs/06-advanced-firmware-driver-mods.md).

## ⚖️ الترخيص والاستخدام المسؤول

- التعديلات اشتقاقية من كود **GPL-2.0** (النواة) وكود **BSD/GPL** (open-ath9k-htc-firmware) — انظر [LICENSE](LICENSE).
- knob القدرة (`txpower`) موجود للقياسات وبحث ميزانية الرابط — الالتزام بالحدود التنظيمية المحلية مسؤوليتك؛
  الوثائق تعتبر تجاوز القيم المعايرة خارج نطاق العمل.

📅 الحالة: 2026-11-15 · نواة 6.12.117 · فيرموير 1.5 (51,144 بايت) · درايفر v4.1 · مجموعة الاختبار 51 PASS / 2 FAIL موثقين
