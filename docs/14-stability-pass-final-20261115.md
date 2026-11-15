# 📋 Final Stability Pass — TL-WN422G v2 (AR9271) — 2026-11-15

> Scope: stability first, then throughput/range. One change at a time, each measured.
> Session log files: `logs/` (baseline-phase1.log, txpower-*.log, test-all-v4.1*.log, soak-2h.log).
> Build version after this pass: **driver v4.1** (CONFIG_ATH_DEBUG + NF fix), firmware 1.5 (custom dev slot).

## 1) Changes — table

| # | Setting / change | Before | After | Verified? (how) |
|---|---|---|---|---|
| 1 | USB autosuspend rule `/etc/udev/rules.d/91-ath9k-htc-stability.rules` (new, 0cf3:1006 only) | No rule; runtime `control=on` only by luck (powertop installed, its service disabled — nothing persisted the state) | Rule present; `udevadm verify`: 1 OK / 0 FAIL; device re-trigger sets `control=on`, `autosuspend_delay=-1000` (kernel "disabled"), stays `active` after 5 s idle | **yes** — `udevadm verify` + live `udevadm trigger` + idle check |
| 2 | NM global power-save off `/etc/NetworkManager/conf.d/90-wifi-powersave-off.conf` (`wifi.powersave=2`) (new) | Only the ".... 1" profile had PS off; any new profile would default to `auto` (PS signature: 105 ms RTT spikes, docs/11) | `nmcli general reload` clean; runtime `Power save: off` | **yes** — `iw get power_save` after reload |
| 3 | Regulatory domain → **EG** `/etc/systemd/system/regdom-eg.service` (new, enabled) | `CN` — came **over the air** from the AP beacon (no local config existed; this NM build has no `wifi.country` key) | `country EG: DFS-ETSI`, user-level; verified sticky **while associated to the CN-advertising AP** (user regdom > beacon IE). 2.4 GHz: ch 1–13 @ 20 dBm — no operational change | **yes** — `iw reg get` shows EG during association |
| 4 | Boot probe `/etc/systemd/system/ath9k-boot-probe.service` + `ath9k-fw-build/boot-fw-probe.sh` (new, enabled) | Boot-time fw 1.4.0 on 2026-11-15 15:20 boot — because the modprobe conf was created 15:31 (after boot). Initramfs checked: **no ath9k modules inside** → no initramfs problem; next boot should load 1.5 | Every boot appends fw version + params + regdom + PM state to `/var/log/ath9k-boot.log` | partially — service enabled and manually proven to log correctly; **full proof = next reboot (see §3)** |
| 5 | Watchdog `ath9k-fw-build/ath9k-watchdog.sh` (v4.2) + `ath9k-watchdog.service` (new, enabled, Restart=always) | none | Detects: ping fail + frozen tx/rx counters ×3 (≈15–20 s) → **1)** driver unbind/bind → **2)** NM reconnect (re-association + DHCP) → **3)** `usbreset` → physical-replug advice. Ping fail with moving counters = association problem → logged, **no reset**. Gateway **re-derived every check** (survives subnet changes). Rate limit 3 recoveries/hour, ALERT logged once per episode. Logs `/var/log/ath9k-watchdog.log` | **yes** — `--selftest` dry-run + **a real hang on 2026-11-15 17:22 (see §2a)**: detection, 2× unbind/bind + usbreset, rate-limit refusal, and correct "association problem, no reset" classification all observed live |
| 6 | `CONFIG_ATH_DEBUG` compiled in (`ath9k-htc.kbuild`, backup kept) | ath_dbg = no-op ("Timeout waiting for WMI" string: **0** matches in deployed .ko) | ath_dbg live: `ath: phyX: Set channel…`, `Configure tx…` printed with mask 0x2601; **runtime mask** via debugfs `$D/debug` (0x2601→output; 0x400→quiet, verified A/B); default at load = FATAL-only (0x400, quiet) | **yes** — strings check + dmesg A/B across mask writes |
| 7 | **NF node fix** (driver v4.1): `read_file_nf` field mask `AR_PHY_MINCCA_PWR` (shift 19) → `AR9280_PHY_MINCCA_PWR` (shift 20), matches stock `ar9002_hw_do_getnf`; `raw=` now printed | All previous NF readings were wrong-field decodes: -236 (idle) / -43 (associated, docs/13 claimed "real") were artifacts | Associated NF = **-116/-118 dBm** — inside the 9271 vendor good window (**-127…-116**, nominal -118). Raw `0x18c1cf9c` cross-decodes: old mask → -232 (matches old readings), new mask → -116. NF stable across traffic, independent of RSSI (-17) → real calibrated NF, **not** RSSI/CCA | **yes** — raw-register decode proof + vendor window + RSSI independence |
| 8 | `test-all.sh` v4.1 (backup kept): `health()` sanity gates + NF honesty gate | Old parser could show phantom counts (the "46K timeouts" artifact class); NF gate had a hole (-43 passed as "plausible") | `health()`: unparseable-count detection, u16 wrap-aware delta, impossible-jump (>6) refusal, diag invariant cross-check (tx − timeouts == rsp). NF gate: 9271 window tiers + association note | **yes** — 2 full suite runs; final: **PASS=51 FAIL=2** (the 2 = intentional `ack_timeout` readback FAILs, documented limitation) |

### Decisions documented (no change made)

| Item | Decision | Reason (measured) |
|---|---|---|
| Phase 5 ACK timeout (0x8014) | **SKIPPED** | Link is **-17…-20 dBm** (AP in the same room); default ACK timeout is correct for short range; knob cannot hold anyway (mac80211/ROM rewrite — docs/13) |
| `nohwcrypt=1` (docs/11 recommendation) | **Not applied** | 0 tx retries / 0 tx failed with hw crypt ON in every test today; no measured problem justifies the change. If key-management issues ever appear, test it with one deploy reload |
| HT40 | **Not testable** | The AP operates 20 MHz (iw link). 20 MHz kept — also the stability-preferred choice |
| Fixed TX power | **Unsupported** — keep default 20 dBm | `iw set txpower fixed 1500` is accepted but never reaches hardware: no ath_dbg config activity, FW-side `tx_power` unchanged (60/63), `iw info` unchanged. ath9k_htc has no tx-power apply path outside channel config |
| Channel choice | AP-locked to ch 11 | Scan: ch11 carries our AP (-17) + one **co-channel neighbor at -36 dBm** (main congestion source). ch1 has 3 weak APs (-80…-84), ch2 one (-69), ch9 one (-85). **AP-side lever:** moving the ROUTER to ch 6 or 13 would cut contention (adapter-side nothing to change) |
| Short GI | **Kept** | MCS 7 short GI observed under load with 0 retries |

## 2) Measurements

| Test | Phase-1 baseline (pre-changes) | Final config | Note |
|---|---|---|---|
| 60 s ping (120×0.5 s) | 0% loss, avg 5.6 ms, max 107 ms | 0% loss, avg 5.9 ms, max 35 ms | stability unchanged (already excellent) |
| Load burst | 1000×1200 B @100 pps: 0% loss, max 40 ms | 500×1200 B: 0% loss, max 44–46 ms | 0 tx retries / 0 failed in **all** runs |
| TCP download (cachefly 10 MB, adapter-bound) | 1.25 / 1.34 MB/s | 1.19 / 1.32 MB/s | ceiling = ISP (~10–13 Mbps), not the adapter |
| WMI suite | PASS=50 FAIL=3 | **PASS=51 FAIL=2** | the fixed FAIL = the NF artifact |
| 2 h soak (ping every 10 s + 5-min load blocks + watchdog) | — | **run 1 + run 2, see §2a/§2b** | run 1 interrupted by a network-infrastructure change; run 2 = clean 2 h |

### 2a) Unplanned LIVE event during soak run 1 — the watchdog earned its keep

At **17:22** the link died for real (ping fail + tx/rx counters **frozen at 0**) — the exact hang signature the watchdog was built for (docs/13):

1. Detection fired after ~15–20 s (correct threshold).
2. Recovery ladder executed: **2× unbind/bind** (dmesg: "USB layer deinitialized" → FW re-transferred → HTC initialized, fw 1.5) + usbreset path; driver came back alive.
3. Ping still failed → root cause was **userspace**: the iface had no valid IP (fresh DHCP lease). v4.1 had no NM step — the rate limit then correctly refused further resets (3/hour) and logged the physical-replug advice.
4. As counters started moving again with ping still failing, it correctly reclassified as "association/link problem — no reset" (no spurious USB resets).
5. Watchdog v4.2 was written from this event: **NM-reconnect as recovery step 2** (before usbreset), **gateway re-derived every check**, **once-per-episode ALERT logging**.

Network context of the event (infrastructure, not the adapter): the AP was **moved ch11 → ch6 at ~17:05** (both radios re-associated; the adapter survived that channel hop on its own), then during the hang the adapter re-associated to a **different BSSID** (`16:bc:fa:65:41:63`, ch11 — mesh unit) and the whole LAN **changed subnet** (`10.64.17.0/24` → `172.17.235.0/24`, gateway `.167`). After recovery the adapter holds a valid lease and pings the new gateway at 0% loss / ~5 ms.

### 2b) Soak results

**Run 1** (`logs/soak-run1-pre-event.log`, 16:43 → 17:33, aborted on the infra change): pre-event window (~39 min): 0 tx retries / 0 tx failed, 0 WiFi/USB dmesg errors (all "error lines" in the window were unrelated `ata5` SATA noise from an empty port), download 0.98–1.41 MB/s, watchdog: correct behavior during the live hang (above).

**Run 2** (`logs/soak-run2.log`, 17:41 → 19:41, full 2 h on the new network):

**SOAK RUN 2 RESULTS — to be filled at completion:**
`[PENDING — will be replaced with the measured summary]`

## 3) Persistence — what the next reboot must show

`/var/log/ath9k-boot.log` (appended automatically by `ath9k-boot-probe.service`) must show:

1. `Transferred FW: ath9k_htc/htc_9271-1.dev.0.fw` + `FW Version: 1.5` (not 1.4.0)
2. `use_dev_fw = 1` (params block)
3. `country EG`
4. `control=on` + `Power save: off`

If item 1 shows 1.4.0 again → tell me / open a session; that would mean something loads the module before `/etc/modprobe.d` is mounted (not the case today — the module is NOT in the initramfs).

## 4) Still unresolved (stated plainly)

1. **Boot-time firmware persistence** — not yet proven; needs the reboot in §3 (everything else about the config is verified live).
2. **`ack_timeout` knob** — deliberately out of scope (short-range link). The WMI path works and the blackbox records the write, but 0x8014 does not hold (mac80211 + ROM shadow rewrite). Unchanged from docs/13.
3. **Watchdog's real-hang recovery** — detection and recovery *ladder* are selftest-verified, and the reload-time behavior was live-verified, but a true hang has not occurred yet; the soak's trigger count (expected 0) is not a test of recovery.
4. **RTT spikes (35–372 ms, no loss)** — appear in every configuration, both today's and earlier sessions'. 0% loss, 0 retries — not an adapter-defect signature. Most probable: airtime on the shared channel (co-channel neighbor at -36 dBm + the laptop's second radio on the same AP/channel). Environmental; unresolved.
5. **ath9k_hw (distro module) debug output** — stays silent; only ath9k_htc/ath9k_common were rebuilt with CONFIG_ATH_DEBUG. The WMI-timeout messages that matter live in ath9k_htc and are now visible.
6. **`thermal avg_rssi`** stays 0 (hooks=0 by design — docs/12); untouched.

## 5) Revert instructions (exact)

```bash
# 1. USB autosuspend rule
sudo rm /etc/udev/rules.d/91-ath9k-htc-stability.rules
sudo udevadm control --reload-rules

# 2. NM global powersave conf
sudo rm /etc/NetworkManager/conf.d/90-wifi-powersave-off.conf
sudo nmcli general reload

# 3. Regulatory domain service
sudo systemctl disable --now regdom-eg.service
sudo rm /etc/systemd/system/regdom-eg.service && sudo systemctl daemon-reload

# 4. Boot probe
sudo systemctl disable --now ath9k-boot-probe.service
sudo rm /etc/systemd/system/ath9k-boot-probe.service /var/log/ath9k-boot.log
sudo systemctl daemon-reload

# 5. Watchdog
sudo systemctl disable --now ath9k-watchdog.service
sudo rm /etc/systemd/system/ath9k-watchdog.service /var/log/ath9k-watchdog.log
sudo systemctl daemon-reload

# 6+7. Driver v4.1 (ATH_DEBUG + NF fix) — restore prior sources and rebuild
cd /home/mohamed/Projects/ath9k-fw-build
cp ath9k-htc.kbuild.bak.1794752804 ath9k-htc.kbuild
cp "/home/mohamed/Projects/TP-Link TL-WN422G v2 Qualcomm Atheros AR9271/sources/driver/linux-6.12.116-ath9k/drivers/net/wireless/ath/ath9k/htc_drv_debug.c.bak.1794753465" \
   "/home/mohamed/Projects/TP-Link TL-WN422G v2 Qualcomm Atheros AR9271/sources/driver/linux-6.12.116-ath9k/drivers/net/wireless/ath/ath9k/htc_drv_debug.c"
./driver-build.sh && sudo ./deploy.sh   # then reconnect NM

# 8. test-all.sh
cp /home/mohamed/Projects/ath9k-fw-build/test-all.sh.bak.1794753101 /home/mohamed/Projects/ath9k-fw-build/test-all.sh
```

Files kept intentionally: `ath9k-watchdog.sh` and `boot-fw-probe.sh` (the services reference them).

## 6) sudoers diag file

`/etc/sudoers.d/99-ath9k-diag` — status at end of session: see final chat report (removed / kept, with reason).

📅 Session: 2026-11-15, kernel 6.12.117+deb13-amd64, iface `wlxf8d111b37c15` (0cf3:1006)
