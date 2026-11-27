#!/usr/bin/env bash
# probe-reply-capacity.sh — decisive probes for the v3 reply-path death
#
# Pattern observed: exactly N successful WMI replies after each fw load
# (N=2 with the v3 build, N=1 with the old build that fired one boot
# telemetry event). Suspects:
#   (a) CMD_REPLY pool = 2 buffers (ATH_WMI_MAX_CMD_REPLY, wlan_cfg.h:50)
#       never recycled post-boot
#   (b) target CTL endpoint credits exhausted (host-needs-credit state)
#
# RUN WITH:  sudo deploy.sh   (fresh fw load!)   then:  sudo ./probe-reply-capacity.sh
set -u

IFACE=$(find /sys/class/net -mindepth 1 -maxdepth 1 -name 'wlx*' -printf '%f\n' 2>/dev/null | sort | head -1)
[ -z "$IFACE" ] && { echo "interface missing — replug/reload first"; exit 1; }
D=/sys/kernel/debug/ieee80211/phy$(cat "/sys/class/net/$IFACE/phy80211/index")/ath9k_htc

say() { printf '  %s\n' "$*"; }

try_cat() { # $1 = file, prints OK/FAIL verdict
    local out
    out=$(timeout 8 cat "$D/$1" 2>&1 | head -2)
    if echo "$out" | grep -qi "timed out\|no such file\|permission denied"; then
        echo "FAIL: $out" | head -1
    else
        echo "OK: $(echo "$out" | head -1 | cut -c1-64)"
    fi
}

echo "== 0) enable WMI timeout debug prints (driver-local, no WMI) =="
echo 0x00002000 | tee "$D/debug" >/dev/null 2>&1 && say "ATH_DBG_WMI mask on" || say "(debug mask write failed — dmesg will be quiet about timeouts)"

echo
echo "== 1) capacity test: rssi_hist at t=0 / +3s / +6s =="
say "read 1: $(try_cat rssi_hist)"
sleep 3
say "read 2: $(try_cat rssi_hist)"
sleep 3
say "read 3: $(try_cat rssi_hist)"

echo
echo "== 1b) delayed-recycle test: wait 15s, then rssi_hist again =="
sleep 15
say "read 4 (after 15s idle): $(try_cat rssi_hist)"

echo
echo "== 2) custom ping AFTER the wedge (REAL round trip + host-side diag) =="
echo "   host: $(timeout 5 cat "$D/wmi_diag" 2>&1 | head -1)"
if echo 4242 | timeout 8 tee "$D/custom_ping" >/dev/null 2>&1; then
    say "ping round trip: OK - counters:"
    timeout 8 cat "$D/custom_ping" 2>&1 | sed 's/^/      /'
else
    say "ping round trip: FAIL (write timed out - target did not reply)"
fi
echo "   host after: $(timeout 5 cat "$D/wmi_diag" 2>&1 | head -1)"

echo
echo "== 3) official WMI still alive? (scan trigger) =="
ip link set "$IFACE" up 2>/dev/null
if timeout 12 iw dev "$IFACE" scan trigger 2>&1; then
    say "scan: accepted (official WMI path alive)"
else
    say "scan: failed/timeout (official WMI path dead too)"
fi

echo
echo "== 4) dmesg tail (look for 'Timeout waiting for WMI command') =="
dmesg | grep -i "wmi\|htc\|ath9k_htc" | tail -25

echo
echo "PROBES DONE"
