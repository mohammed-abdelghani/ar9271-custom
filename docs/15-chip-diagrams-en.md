# 🧩 AR9271 Chip Diagrams — Functional Architecture (English)

> **Scope:** functional architecture of the Qualcomm Atheros **AR9271** as used by this project — verified against the
> driver/firmware source trees in [`sources/`](../patches/) and live measurements (`usbmon`, debugfs, dmesg) documented in
> [10](10-debug-session-log.md), [13](13-wmi-wedge-root-cause.md) and [14](14-stability-pass-final-20261115.md).
> **Arabic version:** [16-chip-diagrams-ar.md](16-chip-diagrams-ar.md)
>
> ℹ️ Board-level schematics (PCB, PA matching, antenna network) are TP-Link proprietary and not published — these diagrams
> cover the chip and firmware/driver architecture instead. All diagrams are [Mermaid](https://mermaid.js.org/) and render
> natively on GitHub.

**Chip identity:** Qualcomm Atheros AR9271 — 802.11 b/g/n 1×1, 2.4 GHz only, USB 2.0 device (`0cf3:1006`),
firmware-resident SoC (no host-side MAC). Internal platform codename **Magpie** (`PROJECT_MAGPIE` in the firmware source);
firmware target name **K2** (`k2_HIF_usb_patch.c`, register docs in [13](13-wmi-wedge-root-cause.md)).
The embedded core is an **Xtensa** processor running the open-ath9k-htc-firmware (our custom build: v1.5, 51,144 bytes).

---

## 1) System overview — host to air

```mermaid
flowchart LR
    subgraph HOST["Host PC — Debian Linux, kernel 6.12"]
        NM["NetworkManager<br/>wpa_supplicant"]
        MAC80211["mac80211 / cfg80211"]
        DRV["ath9k_htc driver<br/>custom v4.1"]
        UDEV["host services:<br/>watchdog · boot-probe · regdom"]
        NM --- MAC80211 --- DRV
        UDEV -. monitors .-> DRV
    end
    subgraph USB["USB 2.0 bus"]
        EP0["EP0<br/>control / enumeration"]
        EPC["bulk endpoints<br/>see §3"]
    end
    subgraph CHIP["AR9271 SoC (Magpie/K2)"]
        FW["custom firmware v1.5<br/>46 WMI commands"]
        RADIO["MAC · Baseband · RF 2.4 GHz"]
    end
    ANT["RP-SMA antenna<br/>4 dBi external"]
    DRV <--> EPC
    EPC <--> FW
    FW <--> RADIO <--> ANT
```

---

## 2) AR9271 SoC internal blocks

```mermaid
flowchart TB
    subgraph SOC["AR9271 — Magpie platform, K2 target"]
        direction TB
        subgraph CPU["Xtensa embedded core"]
            ROM["ROM<br/>bootstrap + MAC shadow registers<br/>re-written on every sleep/wake"]
            IRAM["IRAM segment — 39,744 B budget<br/>custom .text = 35,467 B (0x8A8B)<br/>margin ≈ 3,709 B"]
            DRAM["DRAM — 122.4 KB address space<br/>data + bss ≈ 117.5 KB<br/>margin ≈ 2,208 B"]
        end
        WMAC["WLAN MAC<br/>802.11n HT20/HT40 · MCS 0-7"]
        BB["Baseband<br/>OFDM + CCK"]
        RF["RF 2.4 GHz<br/>PA + LNA"]
        USBDC["USB 2.0 device controller<br/>bulk EP1-EP6 + EP0"]
        EEPROM["EEPROM<br/>calibration: PDACC power table · opflags"]
        TICK["firmware tick — 2 s periodic chores<br/>thermal governor · auto-ANI · telemetry"]
        BBX["black box ring buffer<br/>16 post-mortem events"]
    end
    EEPROM -. calibration at init .-> RF
    CPU <--> WMAC <--> BB <--> RF
    CPU --- TICK --- BBX
    USBDC <--> CPU
```

Memory budget measured on our builds (docs [8](08-firmware-resident-features.md), [12](12-thermal-batch.md), [13](13-wmi-wedge-root-cause.md)):
every new firmware feature is budgeted against the **IRAM margin (≈3.7 KB)** — this is why the airtime feature stays deferred.

---

## 3) USB endpoint map

Verified from: firmware `hif_usb.h` pipe table, driver `htc_drv_init.c` service list, and **usbmon capture** of the WMI
wedge ([13](13-wmi-wedge-root-cause.md)).

| USB endpoint | Direction | Carries | Verified by |
|---|---|---|---|
| EP0 | bidir | USB control / enumeration, `usbreset` | standard |
| **EP4 OUT** | host → target | **WMI commands** (HIF `PIPE_COMMAND` = 4, max msg 512 B on K2) | usbmon `SUB-OUT ep4` |
| **EP3 IN** | target → host | **WMI responses + credit/interrupt reports** (HIF pipe 3) — *total wire size must stay < 64 B, see §6* | usbmon `RCV-IN ep3` |
| EP2 IN | target → host | WLAN RX frames (HIF `PIPE_RX` = 2; firmware sends via HTC `RX_ENDPOINT_ID`) | fw source `if_ath.c:69` |
| EP1 / EP5 / EP6 OUT | host → target | WLAN TX pipes (HIF `TX` = 1, `HP_TX` = 5, `MP_TX` = 6) — beacon / CAB / mgmt / data-QoS services | fw `hif_usb.h` |

HTC services negotiated at init (driver `htc_drv_init.c` — 9 services, 33 credits):
WMI control · beacon · CAB · UAPSD · mgmt · data BE/BK/VI/VO. Endpoint↔pipe mapping is negotiated in the HTC
connect/config-pipe exchange at init; the two rows above are pinned by live usbmon evidence.

```mermaid
flowchart LR
    subgraph D2["driver"]
        WMI["wmi.c<br/>command queue"]
        TXRX["htc_drv_txrx.c<br/>TX/RX rings"]
    end
    subgraph U2["USB"]
        E4["EP4 OUT"]
        E3["EP3 IN"]
        E2["EP2 IN"]
        E1["EP1/5/6 OUT"]
    end
    subgraph F2["firmware"]
        DISP["WMI dispatcher<br/>wmi_svc.c"]
        RXQ["RX tasklet<br/>+ macfilter · rssi_hist · netflow"]
        TXQ["TX tasklet<br/>+ rate control · tx-power effective"]
    end
    WMI -- "cmd ≤ 512 B" --> E4 --> DISP
    DISP -- "rsp &lt; 64 B total!" --> E3 --> WMI
    TXQ -- frames --> E1
    E2 -- frames --> RXQ
```

---

## 4) TX path — host to air

```mermaid
sequenceDiagram
    participant H as mac80211
    participant D as ath9k_htc driver
    participant U as USB EP1/5/6 OUT
    participant F as firmware TX tasklet
    participant R as MAC → Baseband → RF

    H->>D: skb (tx info: rate idx, flags)
    D->>D: HTC header + TX descriptor<br/>ath9k_htc_tx_start
    D->>U: frame
    U->>F: frame + descriptor
    F->>F: rate control 11n lookup<br/>(rc_profile / rc_fixed runtime knobs)
    F->>F: TX power = custom_tx_power_effective()<br/>(thermal governor cap, safety floor 20)
    F->>R: transmit series (tries per rate)
    R-->>F: TX status
    F-->>U: send-complete → credits released
    U-->>D: TX status event → mac80211
```

Custom firmware hooks touched in this path (all runtime-tunable, [7](07-feature-port-roadmap.md), [12](12-thermal-batch.md)):
`txpower` (PDACC index override), `rc_profile` (tries/rate series), `rc_fixed` (fixed-rate toggle),
thermal governor (link-quality cap 48 / weak cap 60 / manual cap / load backoff — bit1 off by default).

---

## 5) RX path — air to host

```mermaid
sequenceDiagram
    participant R as RF → Baseband → MAC
    participant F as firmware RX tasklet
    participant U as USB EP2 IN
    participant D as ath9k_htc driver
    participant H as mac80211

    R->>F: received frame (desc: RSSI, NF, phy errs)
    F->>F: MAC filter check (allow/deny list,<br/>drops before USB traffic)
    F->>F: RSSI histogram bin · netflow counters<br/>(custom_rx_hooks, default off — see 12/13)
    F->>U: frame → EP2 IN
    U->>D: skb + RX status
    D->>D: spectral capture tap<br/>(ath9k_common spectral)
    D->>H: ieee80211_rx_irqsafe
```

The RX-side custom hooks default **off** (`custom_rx_hooks=0`) — the stable baseline. The "dies after ~2 custom replies"
mystery that forced this off was root-caused and **fixed** ([13](13-wmi-wedge-root-cause.md)): it was never the hooks,
it was 64-byte WMI control packets wedging the USB controller (§6).

---

## 6) WMI command round-trip — and the 64-byte rule

```mermaid
sequenceDiagram
    participant D as driver (wmi.c)
    participant T as target (wmi_svc.c → handler)
    autonumber
    D->>T: WMI cmd on EP4 OUT (HTC+WMI header + payload)
    T->>T: dispatch by cmd ID<br/>custom IDs 0x21-0x2F
    T-->>D: response on EP3 IN<br/>HTC hdr + WMI hdr + payload + credit trailer
    D->>D: wmi_diag: issued − timeouts = replies
    Note over D,T: ⚠️ RULE: total EP3 packet &lt; 64 B on the wire.<br/>Exactly 64 B wedges the USB controller —<br/>reply reaches the host, then EP4 stops ACKing and<br/>EP3 floods EOVERFLOW every ~2 ms (usbmon-proven).<br/>Firmware caps the credit trailer (≤ 60 B, HTC_EP3_SAFE_PACKET).
```

The three commands that historically crossed the limit (rssi_hist 48-bin, chan_survey s16-NF, netflow) were all
shrunk in firmware **and** driver together ([13](13-wmi-wedge-root-cause.md), fixes 1-3) — zero WMI timeouts since.

---

## 7) Firmware memory map (custom build v1.5)

```mermaid
flowchart TB
    subgraph M["target memory"]
        ROM["ROM<br/>bootstrap · MAC register shadow<br/>(re-written by mac80211-induced wake —<br/>why ack_timeout 0x8014 cannot hold)"]
        subgraph IRAM["IRAM 39,744 B"]
            TXT[".text 35,467 B — 0x8A8B<br/>(46 WMI handlers + 13 features)"]
            MARGIN1["free ≈ 3,709 B"]
        end
        subgraph DRAM["DRAM 122.4 KB"]
            DATA["data + bss ≈ 117.5 KB<br/>(statics: stats, filters, ring buffers)"]
            MARGIN2["free ≈ 2,208 B"]
        end
        EEPROM2["EEPROM<br/>PDACC tx-power table · opflags · cal"]
    end
    ROM -. patch region limit 39,680 B .-> IRAM
```

Binary: `htc_9271-1.dev.0.fw` = **51,144 bytes** (loaded with `use_dev_fw=1`, custom slot — stock `htc_9271-1.4.0.fw` untouched).

---

## 8) Where each feature lives

| Feature | Lives in | debugfs interface | Doc |
|---|---|---|---|
| Custom WMI ping | firmware + driver | `custom_ping` | [13](13-wmi-wedge-root-cause.md) |
| Rate-control profiles / fixed rate | firmware | `rc_profile` / `rc_fixed` | [7](07-feature-port-roadmap.md) |
| TX power override (PDACC) | firmware | `txpower` | [7](07-feature-port-roadmap.md) |
| Rate usage + time-on-air | firmware RC | `rate_usage` | [8](08-firmware-resident-features.md) |
| Reverse telemetry event (0x1008) | firmware → driver | `telemetry` | [8](08-firmware-resident-features.md) |
| Auto-ANI v2 (firstep + CCA ±3 dB) | firmware tick | `ani_auto` | [8](08-firmware-resident-features.md), [9](09-firmware-v3-batch.md) |
| RSSI histogram (64 bins) | firmware RX hook | `rssi_hist` | [8](08-firmware-resident-features.md) |
| Radio-side MAC filter | firmware RX tasklet | `macfilter` | [8](08-firmware-resident-features.md) |
| Block-ACK window (1-64) | firmware | `baw` | [9](09-firmware-v3-batch.md) |
| ACK timeout (0x8014) | firmware reg-write | `ack_timeout` (does not hold — see [13](13-wmi-wedge-root-cause.md)) | [9](09-firmware-v3-batch.md) |
| Channel NF survey | firmware + driver notify | `chan_survey` | [9](09-firmware-v3-batch.md) |
| Mini-netflow (6 MACs) | firmware RX hook | `netflow` | [9](09-firmware-v3-batch.md) |
| Auto-spectral on PHY-error burst | firmware | `auto_spectral` | [9](09-firmware-v3-batch.md) |
| Black box (16 events) | firmware | `blackbox` | [9](09-firmware-v3-batch.md) |
| Thermal governor + cap | firmware tick (2 s) | `thermal` | [12](12-thermal-batch.md) |
| Spectral scan (FFT 56 bins) | driver `ath9k_common` + HW | `spectral_scan0` + ctl | [7](07-feature-port-roadmap.md) |
| NF live read (AR9280 mask, v4.1) | driver reg-read via WMI | `nf` | [14](14-stability-pass-final-20261115.md) |
| WMI diagnostics | driver | `wmi_diag` | [13](13-wmi-wedge-root-cause.md) |
| Hang watchdog (recover ladder) | host systemd | `ath9k-watchdog.service` | [14](14-stability-pass-final-20261115.md) |

---

📅 Generated from the project's own source trees + live measurements, 2026-11-15 · kernel 6.12.117 · firmware 1.5 (51,144 B) · driver v4.1
