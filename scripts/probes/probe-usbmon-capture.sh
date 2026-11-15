#!/usr/bin/env bash
# probe-usbmon-capture.sh — decisive discriminator for the WMI wedge:
# does the TARGET put the rssi_hist reply on the USB bus at all?
#   reply bytes seen on EP4 IN  -> target sent it, HOST dropped it (host bug)
#   no reply bytes on EP4 IN    -> target never sent it (target-side death)
#
# RUN WITH: sudo ./probe-usbmon-capture.sh
set -u
B="${BUILD_DIR:-$HOME/ath9k-fw-build}"
MON=/tmp/usbmon-capture.txt

[ "$(id -u)" -eq 0 ] || { echo "run with sudo: sudo $0"; exit 1; }

# 1) fresh load, NM isolated
"$B/probe-fresh-reload.sh" || exit 1

IFACE=wlxf8d111b37c15
[ -d "/sys/class/net/$IFACE" ] || IFACE=$(ls /sys/class/net/ | grep '^wlx' | head -1)
N=$(cat "/sys/class/net/$IFACE/phy80211/index")
D="/sys/kernel/debug/ieee80211/phy$N/ath9k_htc"
DEVNUM=$(cat "/sys/bus/usb/devices/2-1.2/devnum" 2>/dev/null)
BUSNUM=$(cat "/sys/bus/usb/devices/2-1.2/busnum" 2>/dev/null)
echo "capture target: bus=$BUSNUM dev=$DEVNUM phy=$N"

modprobe usbmon 2>/dev/null
[ -e "/sys/kernel/debug/usb/usbmon/${BUSNUM}u" ] || { echo "FAIL: usbmon not available"; exit 1; }

# 2) capture bus traffic while doing the wedge sequence
: > "$MON"
(timeout 40 cat "/sys/kernel/debug/usb/usbmon/${BUSNUM}u" > "$MON" 2>/dev/null) &
MONPID=$!
sleep 0.7

echo "-- op1: custom ping (expect OK)"
echo 111 > "$D/custom_ping" && echo "op1 ping: OK" || echo "op1 ping: FAIL"
sleep 0.5

echo "-- op2: rate_usage (expect OK)"
timeout 8 cat "$D/rate_usage" >/dev/null 2>&1 && echo "op2 rate_usage: OK" || echo "op2 rate_usage: FAIL"
sleep 0.5

echo "-- op3: rssi_hist (historical killer)"
timeout 8 cat "$D/rssi_hist" >/dev/null 2>&1 && echo "op3 rssi_hist: OK" || echo "op3 rssi_hist: FAIL"
sleep 0.5

echo "-- op4: custom ping again (channel alive?)"
echo 222 > "$D/custom_ping" && echo "op4 ping: OK" || echo "op4 ping: FAIL"
sleep 1

kill "$MONPID" 2>/dev/null; wait "$MONPID" 2>/dev/null
echo "wmi_diag: $(cat "$D/wmi_diag")"
echo "capture: $MON ($(wc -l < "$MON") lines, dev $DEVNUM)"
