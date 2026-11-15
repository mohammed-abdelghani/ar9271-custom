#!/usr/bin/env bash
# reset-target.sh — software USB port reset of the AR9271 target
# (equivalent to a physical replug: forces the target back to its
#  boot-ROM download mode after a wedged firmware).
# RUN WITH: sudo ./reset-target.sh
set -u

WEDGED=0
for d in /sys/bus/usb/devices/2-1.*; do
	[ -f "$d/idVendor" ] || continue
	if grep -q 0cf3 "$d/idVendor" 2>/dev/null; then
		DEVP="$d"
		break
	fi
done

if [ -z "${DEVP:-}" ]; then
	echo "target not found on USB — is the card plugged in?"
	exit 1
fi

echo "target at: $DEVP (devnum $(cat "$DEVP/devnum" 2>/dev/null))"

echo "== 1) software port reset (authorized toggle) =="
echo 0 > "$DEVP/authorized"
sleep 2
echo 1 > "$DEVP/authorized"
sleep 3

echo "== 2) re-enumeration check =="
NEWDEV=$(for d in /sys/bus/usb/devices/2-1.*; do
	[ -f "$d/idVendor" ] || continue
	grep -q 0cf3 "$d/idVendor" 2>/dev/null && echo "$d" && break
done | head -1)

if [ -z "$NEWDEV" ]; then
	echo "FATAL: target did not re-enumerate — physical replug required."
	exit 1
fi
echo "target re-enumerated at: $NEWDEV"

echo "== 3) deploy =="
bash "$(cd "$(dirname "$0")" && pwd)/deploy.sh"

echo "== 4) focused verification =="
IFACE=$(ls /sys/class/net/ | grep '^wlx' | head -1)
if [ -z "$IFACE" ]; then
	echo "interface still missing — check: dmesg | tail -30"
	exit 1
fi
D=/sys/kernel/debug/ieee80211/phy$(cat "/sys/class/net/$IFACE/phy80211/index" 2>/dev/null)/ath9k_htc

echo 555 | tee "$D/custom_ping" >/dev/null 2>&1
echo "--- custom_ping ---";  cat "$D/custom_ping" 2>&1
echo "--- rate_usage (head) ---"; head -6 "$D/rate_usage" 2>&1
echo "--- rssi_hist (head) ---"; head -8 "$D/rssi_hist" 2>&1
echo "--- ani_auto ---";         cat "$D/ani_auto" 2>&1
echo "--- macfilter ---";        cat "$D/macfilter" 2>&1
echo "--- txpower ---";          cat "$D/txpower" 2>&1
echo "=== RESET+VERIFY DONE ==="
