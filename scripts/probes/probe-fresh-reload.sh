#!/usr/bin/env bash
# probe-fresh-reload.sh — clean reload of the RUNNING driver+fw pair,
# with NetworkManager unmanaged on the device BEFORE the reload so it
# cannot grab the interface or scan during the probe window.
set -u
[ "$(id -u)" -eq 0 ] || { echo "run with sudo: sudo $0"; exit 1; }
IFACE=$(find /sys/class/net -mindepth 1 -maxdepth 1 -name 'wlx*' -printf '%f\n' 2>/dev/null | sort | head -1)
[ -n "$IFACE" ] || { echo "FAIL: no wlx iface"; exit 1; }

echo "== pre-isolate =="
nmcli device set "$IFACE" managed no && echo "NM unmanaged: yes" || echo "NM unmanaged: FAILED"
ip link set "$IFACE" down 2>/dev/null && echo "iface down: yes"

echo "== reload =="
modprobe -r ath9k_htc 2>/dev/null && echo "removed" || echo "remove failed (was not loaded?)"
sleep 1
modprobe ath9k_htc use_dev_fw=1 && echo "loaded with use_dev_fw=1"

UP=0
for _ in $(seq 1 40); do
	if [ -d "/sys/class/net/$IFACE" ]; then UP=1; break; fi
	sleep 0.5
done
[ "$UP" -eq 1 ] || { echo "FAIL: interface did not appear in 20s"; dmesg | tail -15; exit 1; }

# keep NM away for the probe window
nmcli device set "$IFACE" managed no 2>/dev/null || true
ip link set "$IFACE" down 2>/dev/null || true

N=$(cat "/sys/class/net/$IFACE/phy80211/index")
D="/sys/kernel/debug/ieee80211/phy$N/ath9k_htc"
echo "== state =="
ip link show "$IFACE" | head -1
ls "$D" | head -40
echo "READY: iface=$IFACE phy=$N"
