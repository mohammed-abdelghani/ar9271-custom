# 🛠️ Scripts

Operational tooling for the custom build. The build scripts read the **already-patched** sources from
[`../sources/`](../sources/README.md) inside this repo; outputs go to a space-free mirror — override with
`BUILD_DIR` env (default `~/ath9k-fw-build`) — because the upstream firmware Makefile breaks on paths
containing spaces. Scripts auto-detect the `wlx*` interface (=`wlx` + the adapter's USB MAC — constant
per physical adapter); override with `IFACE`/`ATH9K_IFACE` env where supported.

| Script | Purpose | Docs |
|---|---|---|
| `build/driver-build.sh` | Syncs the patched driver sources into the build mirror and builds `ath9k_htc.ko` + `ath9k_common.ko` (spectral enabled) against the running kernel | [06](../docs/06-advanced-firmware-driver-mods.md) |
| `build/fw-build.sh` | Builds the Xtensa toolchain (hashed tarball fetches) then the custom firmware → `htc_9271.fw` | [06](../docs/06-advanced-firmware-driver-mods.md) |
| `build/ath9k-htc.kbuild` | Out-of-tree Kbuild for the ath9k_htc/ath9k_common pair | [06](../docs/06-advanced-firmware-driver-mods.md) |
| `deploy.sh` | Installs modules into `/lib/modules/.../updates/`, installs the dev firmware, reloads with `use_dev_fw=1`, verifies the interface comes up | [06](../docs/06-advanced-firmware-driver-mods.md), [13](../docs/13-wmi-wedge-root-cause.md) |
| `test-all.sh` | Full acceptance suite (v4.1): 16 sections, health gates between sections, honest NF gate — final: **51 PASS / 2 documented FAILs** | [13](../docs/13-wmi-wedge-root-cause.md), [14](../docs/14-stability-pass-final-20261115.md) |
| `rollback.sh` | Restores stock driver + firmware (nothing stock is ever overwritten) | [06](../docs/06-advanced-firmware-driver-mods.md) |
| `reset-target.sh` | Recovers a wedged target (unbind/rebind, authorized toggle) | [10](../docs/10-debug-session-log.md) |
| `apply-stability-tuning.sh` | Applies the host-side stability set: USB autosuspend rule, NM powersave off, regdom, boot probe + watchdog services | [11](../docs/11-host-side-stability-tuning.md), [14](../docs/14-stability-pass-final-20261115.md) |
| `ath9k-watchdog.sh` | Hang watchdog: detection → unbind/bind → NM reconnect → usbreset ladder, rate-limited, `--selftest` included | [14 §2a](../docs/14-stability-pass-final-20261115.md) |
| `boot-fw-probe.sh` | Appends fw version + params + regdom + PM state to `/var/log/ath9k-boot.log` at every boot | [14 §3](../docs/14-stability-pass-final-20261115.md) |
| `soak-2h-v2.sh` | 2-hour stability soak: 10 s pings, 5-min load blocks, dmesg error watch, watchdog trigger count | [14 §2b](../docs/14-stability-pass-final-20261115.md) |
| `udev/91-ath9k-htc-stability.rules` | Disables USB autosuspend for `0cf3:1006` only | [14 §1](../docs/14-stability-pass-final-20261115.md) |
| `probes/probe-wmi-bisect.sh` | Live A/B isolation: run command → target alive? channel alive? | [13](../docs/13-wmi-wedge-root-cause.md) |
| `probes/probe-usbmon-capture.sh` | usbmon capture on EP3/EP4 — the tool that caught the 64-byte wedge | [13](../docs/13-wmi-wedge-root-cause.md) |
| `probes/probe-deploy-capture.sh` | usbmon during deploy (init command storm) | [13](../docs/13-wmi-wedge-root-cause.md) |
| `probes/probe-fresh-reload.sh` | Fresh module reload + first-command probe | [13](../docs/13-wmi-wedge-root-cause.md) |
| `probes/probe-reply-capacity.sh` | Reply-pool/credit capacity probing | [12](../docs/12-thermal-batch.md) |

⚠️ Services (`ath9k-watchdog.service`, `ath9k-boot-probe.service`, `regdom-eg.service`) are installed by
`apply-stability-tuning.sh` — exact revert instructions live in [docs/14 §5](../docs/14-stability-pass-final-20261115.md).
