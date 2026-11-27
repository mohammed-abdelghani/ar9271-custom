# Third-Party Notices

This repository contains upstream code under `sources/`, plus the project's own patches under
`patches/`. This file lists each upstream component, where it lives, and the license that applies.
The authoritative license texts are kept inside each tree — this file is a map, not a replacement.

## Components

### 1. Linux kernel — `ath9k` wireless driver subtree

- **Location:** `sources/linux-6.12.116-ath9k/` (partial extract of kernel.org **v6.12.116**: the
  `drivers/net/wireless/ath/` subtree plus the minimal top-level files the out-of-tree build needs)
- **Upstream:** <https://kernel.org> — stable tree v6.12.116
- **License:** **GPL-2.0** with Linux-syscall-note — see `sources/linux-6.12.116-ath9k/COPYING`
- **Our changes:** 7 files modified (see the manifest in [sources/README.md](sources/README.md)); the
  delta is also in `patches/ath9k-driver-v6.12.116-custom.patch`

### 2. open-ath9k-htc-firmware — AR7010/AR9271 USB firmware

- **Location:** `sources/open-ath9k-htc-firmware/` (clone of
  [qca/open-ath9k-htc-firmware](https://github.com/qca/open-ath9k-htc-firmware), master @ 2026-11-13)
- **License:** mixed, per the project's own `README`, `LICENCE.TXT` and `NOTICE.TXT`:
  - Qualcomm Atheros–owned code: **ClearBSD licence** (`LICENCE.TXT`)
  - Code derived from earlier Atheros work (e.g. Sam Leffler / Errno Consulting): **BSD-style**
    licenses reproduced in `NOTICE.TXT`
  - Tensilica `xtos/xtensa` code: **MIT licence** (relicensed with Tensilica's permission, per
    `NOTICE.TXT`)
  - Three ECoS source files: **GPLv2 with modification/linking caveats** (see the ECOSGPLCOPYRIGHT
    block in `NOTICE.TXT` — linking or use does not bring the rest of the binary under GPLv2)
- **Our changes:** 12 files modified under `target_firmware/` (+ root `Makefile`); the delta is also
  in `patches/open-ath9k-htc-firmware-custom.patch`
- **Built artifact:** `firmware/htc_9271-1.dev.0.fw` is compiled from this tree

### 3. This project's own work

- `patches/` — plain diffs against the two upstream trees above
- `scripts/` — build/deploy/test/watchdog tooling written for this project
- Top-level [LICENSE](LICENSE) — **GPL-2.0**, matching the kernel driver's license

## License-consistency note (for owner review)

The top-level GPL-2.0 is compatible with the firmware tree's permissive parts (ClearBSD, BSD-style,
MIT). The one item worth a deliberate decision is the **ECoS GPLv2-with-caveats** files inside
`open-ath9k-htc-firmware`: their notice states that linking/using them does not subject the rest of
the binary to GPLv2 — this is upstream's own caveat and is preserved unchanged here. No action was
taken; flagged for awareness.
