# 🧬 Sources — modified driver & firmware trees

The **already-modified, working** source trees for this project. What you see here is exactly what was
built, deployed and tested on the hardware (firmware 1.5, driver v4.1 — [docs/14](../docs/14-stability-pass-final-20261115.md)).
The delta against pristine upstream is also recorded as plain diffs in [`../patches/`](../patches/).

## 📂 Layout

| Tree | Upstream origin | Size | Contents |
|---|---|---|---|
| `linux-6.12.116-ath9k/` | [kernel.org](https://kernel.org) **v6.12.116** tarball (extracted) | 5.3 MB | only `drivers/net/wireless/ath/` — the subtree the out-of-tree build needs (`ath.h`, `regd.*`, `ath9k/`, …). The rest of the kernel tree is intentionally not included |
| `open-ath9k-htc-firmware/` | [qca/open-ath9k-htc-firmware](https://github.com/qca/open-ath9k-htc-firmware) (master clone, 2026-11-13) | 6.2 MB | full firmware tree: `target_firmware/` (our build lives here), `sboot/`, `v2/`, docs, licences |

**Excluded from both trees (deliberately):** upstream `.git` metadata, the Xtensa toolchain download
(`toolchain/dl` — fetched + sha256-verified by the build script), and build artifacts.

## ✏️ Modification manifest

### Driver — `linux-6.12.116-ath9k/drivers/net/wireless/ath/ath9k/` (7 files)

| File | What changed |
|---|---|
| `htc_drv_debug.c` | 15 custom debugfs files (`custom_ping`, `rc_profile`, `txpower`, `nf`, `rssi_hist`, `macfilter`, `baw`, `ack_timeout`, `chan_survey`, `netflow`, `auto_spectral`, `blackbox`, `thermal`, `wmi_diag`, …) + verified register read/write with PS-wakeup isolation + **v4.1 NF mask fix** (`AR9280_PHY_MINCCA_PWR`, shift 20) + honest error propagation (no sentinels) |
| `wmi.h` / `wmi.c` | custom WMI command IDs `0x21-0x2F` + structs + `ath9k_wmi_*()` send functions; chan_survey `nf` shrunk to `s8`; netflow entry back to 10 B (64-byte wire limit — [13](../docs/13-wmi-wedge-root-cause.md)) |
| `htc_drv_init.c` | custom debugfs wiring + module params |
| `htc_drv_main.c` | freq-note notify on channel change (fw NF survey), param plumbing |
| `htc.h` | driver-side structs for the custom batch |
| `hif_usb.h` | comments/docs for the K2 pipe map |

### Firmware — `open-ath9k-htc-firmware/target_firmware/` (+ root Makefile) — 12 files

| File | What changed |
|---|---|
| `wlan/if_ath.c` | the 13 custom WMI handlers + dispatch, thermal governor tick (2 s), `custom_tx_power_effective()`, auto-ANI v2, MAC filter, RSSI histogram, netflow, telemetry event `0x1008`, black box, **`_HTC_SendMsg` trailer cap ≤ 60 B (`HTC_EP3_SAFE_PACKET`) — the 64-byte-wedge fix** |
| `wlan/include/wmi.h` | custom command IDs + payload structs (must mirror the driver side exactly) |
| `magpie_fw_dev/target/wmi/wmi_svc.c` (+ `wmi_svc_api.h`) | WMI service plumbing + diag counters |
| `magpie_fw_dev/target/htc/htc.c` | EP3 safe-packet cap in the send path |
| `magpie_fw_dev/target/buf_pool/buf_pool_static.c` | pool sizing |
| `wlan/if_owl.c` | TX descriptor uses the effective (governor-capped) power index |
| `wlan/ratectrl_11n_ln.c` | rate-usage counters + runtime profile variables + time-on-air |
| `wlan/include/{k2,magpie}/wlan_cfg.h`, `wlan/include/wlan_hdr.h` | build switches for the custom batch |

## 🔨 Building

You do **not** need to apply anything — the trees above are already patched. From the repo root:

```bash
./scripts/build/driver-build.sh    # needs linux-headers-$(uname -r) → ath9k_htc.ko + ath9k_common.ko
./scripts/build/fw-build.sh        # builds the Xtensa toolchain, then htc_9271.fw
```

Both scripts sync these sources into a **space-free mirror** (`BUILD_DIR` env override, default
`~/ath9k-fw-build`) because the firmware Makefile breaks on paths containing spaces. Deployment, testing
and rollback: see [`../scripts/README.md`](../scripts/README.md).

If you prefer to start from pristine upstream, apply the diffs instead — they apply cleanly to
kernel v6.12.116 and the upstream firmware master:

```bash
git clone --branch v6.12.116 --depth 1 \
    https://git.kernel.org/pub/scm/linux/kernel/git/stable/linux.git linux
git -C linux apply ../patches/ath9k-driver-v6.12.116-custom.patch
git clone https://github.com/qca/open-ath9k-htc-firmware.git
git -C open-ath9k-htc-firmware apply ../patches/open-ath9k-htc-firmware-custom.patch
```

⚠️ **Rule of the project:** driver-side and firmware-side changes are always symmetrical (same WMI IDs,
same struct sizes/order). Changing one side alone can wedge the target — read
[docs/13](../docs/13-wmi-wedge-root-cause.md) and [docs/10](../docs/10-debug-session-log.md) first.

📅 Snapshot: 2026-11-15 · firmware 1.5 (51,144 B) · driver v4.1 · kernel 6.12.117 target
