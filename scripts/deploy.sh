#!/usr/bin/env bash
# deploy.sh — install the custom ath9k_htc module + custom dev firmware.
#
# RUN WITH:  sudo ./deploy.sh
#
# Safety model:
#  * The module is installed into /lib/modules/$KVER/updates/ which SHADOWS
#    the distro copy — the original file is never touched.
#  * The firmware is installed as htc_9271-1.dev.0.fw (the "development"
#    slot) — the stable htc_9271-1.4.0.fw is never touched. The module loads
#    it only when use_dev_fw=1 is passed.
#
# CUSTOM BUILD v4-fix (Phase 0): NetworkManager is isolated from the device
# BEFORE the reload (it used to grab the fresh interface, scan and
# re-associate in the middle of the deploy window), a settle wait verifies
# the WMI control path actually answers (custom ping round trip) before
# declaring success, and every check exits non-zero on failure.
set -euo pipefail

KVER=$(uname -r)
B="${BUILD_DIR:-$HOME/ath9k-fw-build}"
# default iface: auto-detect the wlx* interface (wlx + the adapter's USB MAC —
# constant for a given physical stick); override with:  sudo IFACE=wlx... ./deploy.sh
IFACE="${IFACE:-$(find /sys/class/net -mindepth 1 -maxdepth 1 -name 'wlx*' -printf '%f\n' 2>/dev/null | sort | head -1)}"

[ "$(id -u)" -eq 0 ] || { echo "FATAL: run with sudo: sudo $0"; exit 2; }
[ -f "$B/ath9k_htc.ko" ] || { echo "FATAL: missing $B/ath9k_htc.ko (run driver-build.sh first)"; exit 2; }

nm_isolate() {
	# the iface may not exist yet (module not loaded) — resolve lazily
	[ -z "$IFACE" ] && IFACE=$(find /sys/class/net -mindepth 1 -maxdepth 1 -name 'wlx*' -printf '%f\n' 2>/dev/null | sort | head -1)
	[ -z "$IFACE" ] && { echo "-- no wlx* interface yet, skipping NM isolation"; return 0; }
	nmcli device set "$IFACE" managed no >/dev/null 2>&1 \
		&& echo "-- NM unmanaged on $IFACE" \
		|| echo "-- WARN: nmcli unmanaged failed (NM may interfere)"
}

echo "== 1) installing modules into /lib/modules/$KVER/updates/ =="
mkdir -p "/lib/modules/$KVER/updates"
cp "$B/ath9k_htc.ko" "/lib/modules/$KVER/updates/ath9k_htc.ko"
if [ -f "$B/ath9k_common.ko" ]; then
	cp "$B/ath9k_common.ko" "/lib/modules/$KVER/updates/ath9k_common.ko"
	echo "ath9k_common.ko (with spectral scan) installed"
fi
depmod -a "$KVER"

if [ -f "$B/htc_9271.fw" ]; then
	echo "== 2) installing custom firmware as ath9k_htc/htc_9271-1.dev.0.fw =="
	cp "$B/htc_9271.fw" "/lib/firmware/ath9k_htc/htc_9271-1.dev.0.fw"
else
	echo "== 2) custom firmware NOT built yet - stock 1.4.0 stays =="
	echo "   (use_dev_fw=1 is harmless: the driver falls back to 1.4.0)"
fi

echo "== 3) reloading ath9k_htc with use_dev_fw=1 (NM isolated) =="
# existing iface (under its stable name) before removal
nm_isolate
ip link set "$IFACE" down 2>/dev/null || true
modprobe -r ath9k_htc 2>/dev/null || true
sleep 1
modprobe ath9k_htc use_dev_fw=1
nm_isolate   # re-assert: NM must not grab the fresh interface either

echo "== 4) verification (waiting up to 15s for the interface) =="
UP=0
for _ in $(seq 1 30); do
	[ -z "$IFACE" ] && IFACE=$(find /sys/class/net -mindepth 1 -maxdepth 1 -name 'wlx*' -printf '%f\n' 2>/dev/null | sort | head -1)
	if [ -n "$IFACE" ] && [ -d "/sys/class/net/$IFACE" ]; then UP=1; break; fi
	sleep 0.5
done
if [ "$UP" -ne 1 ]; then
	echo "FATAL: INTERFACE NOT FOUND after 15s — probe failed or still pending!"
	dmesg | tail -30
	exit 1
fi

N=$(cat "/sys/class/net/$IFACE/phy80211/index")
D="/sys/kernel/debug/ieee80211/phy$N/ath9k_htc"
if [ ! -d "$D" ]; then
	echo "FATAL: debugfs dir not found at $D (debugfs mounted?)"
	exit 1
fi

echo "== 5) settle: WMI control path must answer (custom ping round trip) =="
SETTLE=0
for _ in $(seq 1 10); do
	# keep NM away while settling
	nmcli device set "$IFACE" managed no >/dev/null 2>&1 || true
	if echo "$((RANDOM))" > "$D/custom_ping" 2>/dev/null; then SETTLE=1; break; fi
	sleep 1
done
if [ "$SETTLE" -ne 1 ]; then
	echo "FATAL: WMI control path did not answer within 10s — target wedged or probe incomplete"
	dmesg | tail -30
	exit 1
fi
echo "-- settle ping OK: $(head -1 "$D/custom_ping")"
echo "-- wmi_diag:       $(cat "$D/wmi_diag")"

echo "--- dmesg (ath9k_htc lines) ---"
dmesg | grep -iE "ath9k_htc|htc_9271" | tail -8
echo "--- interface ---"
ip link show "$IFACE" | head -1
echo "--- debugfs ---"
ls "$D" | head -40

echo "== DEPLOY DONE (NM left UNMANAGED on $IFACE — run test-all.sh, it restores management) =="
