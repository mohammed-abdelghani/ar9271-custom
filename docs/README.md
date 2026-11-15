# 📚 Documentation — TP-Link TL-WN422G v2 (Qualcomm Atheros AR9271) Custom Build

Bilingual documentation set (Arabic + English technical terms) for running this adapter on a **custom-built**
`ath9k_htc` driver + **custom-built** firmware instead of the stock ones. The documentation is a chronological +
thematic record of the whole engineering effort: what was built, how it was debugged, and what was measured.

> **Current build state:** driver **v4.1** (`CONFIG_ATH_DEBUG` + noise-floor mask fix) · firmware **1.5**
> (`htc_9271-1.dev.0.fw`, 51,144 B, 46 WMI commands) · final test suite **51 PASS / 2 documented FAILs** —
> see [14](14-stability-pass-final-20261115.md).

## 🗂️ Index

| # | File | Language | Content |
|---|---|---|---|
| 01 | [01-hardware-specs.md](01-hardware-specs.md) | EN | Full hardware specs of the adapter and the AR9271 |
| 02 | [02-driver-and-firmware.md](02-driver-and-firmware.md) | EN | Driver & firmware on Linux (`ath9k_htc` / `htc_9271-1.4.0.fw` stock) |
| 03 | [03-current-device-status.md](03-current-device-status.md) | EN | Live device snapshot on this machine |
| 04 | [04-usage-and-troubleshooting.md](04-usage-and-troubleshooting.md) | EN | Usage commands, monitor mode, troubleshooting |
| 06 | [06-advanced-firmware-driver-mods.md](06-advanced-firmware-driver-mods.md) | AR | 🧬 Custom build from source — build + deploy + rollback |
| 07 | [07-feature-port-roadmap.md](07-feature-port-roadmap.md) | AR | 🚀 Feature-port roadmap: 11n, spectral scan, WMI runtime tuning |
| 08 | [08-firmware-resident-features.md](08-firmware-resident-features.md) | AR | 🧠 Firmware-resident v2 batch: reverse telemetry, auto-ANI, RSSI histogram, MAC filter |
| 09 | [09-firmware-v3-batch.md](09-firmware-v3-batch.md) | AR | 🧠 v3 batch — 13 firmware-resident features, 46 WMI commands |
| 10 | [10-debug-session-log.md](10-debug-session-log.md) | AR | 🔬 Debug session log — the 4 failure classes, golden rules, recovery tools |
| 11 | [11-host-side-stability-tuning.md](11-host-side-stability-tuning.md) | AR | Host-side stability tuning (powersave, autosuspend, regdom) |
| 12 | [12-thermal-batch.md](12-thermal-batch.md) | AR | 🌡️ Thermal batch v4 — in-firmware heat management (governor + cap) |
| 13 | [13-wmi-wedge-root-cause.md](13-wmi-wedge-root-cause.md) | AR | 🎯 WMI-wedge root cause — proven with usbmon, fixed, fully tested |
| 14 | [14-stability-pass-final-20261115.md](14-stability-pass-final-20261115.md) | EN | 📋 Final stability pass — changes table, measurements, soak, revert instructions |
| 15 | [15-chip-diagrams-en.md](15-chip-diagrams-en.md) | EN | 🧩 AR9271 chip diagrams (Mermaid): SoC blocks, USB endpoint map, TX/RX paths, WMI flow, memory map |
| 16 | [16-chip-diagrams-ar.md](16-chip-diagrams-ar.md) | AR | 🧩 نفس المخططات بالعربية |
| — | [test-final-20261115.log](test-final-20261115.log) | — | Final `test-all.sh` output (the 51 PASS evidence) |

> ℹ️ **Numbering note:** no `05` exists — that document was consolidated into `06` early in the project.
> Original numbering is preserved to keep all internal cross-references (e.g. "docs/13") valid.

## ⚡ Quick facts

| Item | Value |
|---|---|
| **Model** | TP-Link TL-WN422G v2 — 54 Mbps High Gain Wireless USB Adapter |
| **Chipset** | Qualcomm Atheros AR9271 (platform codename Magpie, target K2) |
| **USB ID** | `0cf3:1006` (reported name: `ATHEROS USB2.0 WLAN`) |
| **Wireless** | 802.11b/g — marketing name "54 Mbps" — but officially HT20/HT40 **802.11n MCS 0-7** capable (proven live, [07](07-feature-port-roadmap.md)) |
| **Antenna** | External detachable 4 dBi, RP-SMA |
| **Linux driver** | `ath9k_htc` (in-tree) → custom build v4.1 |
| **Firmware** | stock `htc_9271-1.4.0.fw` → custom v1.5 (`htc_9271-1.dev.0.fw`, 46 WMI commands) |
| **Interface here** | `wlxf8d111b37c15` (MAC `f8:d1:11:b3:7c:15`) |
| **Kernel** | 6.12.117+deb13-amd64 (Debian) |

> 💡 **Don't confuse** this adapter with the **TL-WN722N** — both use the AR9271 and both support monitor mode on Linux,
> but the WN722N is marketed as 802.11n/150 Mbps while the WN422G v2 is marketed as 802.11g/54 Mbps.
> Functionally (per `iw list`) both negotiate 11n rates on 2.4 GHz.

## 📖 Suggested reading order

1. [15/16 — chip diagrams](15-chip-diagrams-en.md) — the architecture map (EN/AR)
2. [06 — custom build](06-advanced-firmware-driver-mods.md) — how the build/deploy pipeline works
3. [07 → 08 → 09](07-feature-port-roadmap.md) — feature batches v1 → v2 → v3
4. [10 → 13](10-debug-session-log.md) — the debugging story: 4 failure classes → 64-byte WMI wedge root cause
5. [12 → 14](12-thermal-batch.md) — thermal management + final stability pass with measurements

📅 Last updated: 2026-11-15
