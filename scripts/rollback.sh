#!/usr/bin/env bash
# rollback.sh — restore the stock distro ath9k_htc module + firmware 1.4.0.
#
# RUN WITH:  sudo ./rollback.sh
set -euo pipefail

KVER=$(uname -r)
IFACE=$(find /sys/class/net -mindepth 1 -maxdepth 1 -name 'wlx*' -printf '%f\n' 2>/dev/null | sort | head -1)

[ "$(id -u)" -eq 0 ] || { echo "Run with sudo: sudo $0"; exit 1; }

echo "== removing custom modules from updates/ =="
[ -n "$IFACE" ] && ip link set "$IFACE" down 2>/dev/null || true
modprobe -r ath9k_htc 2>/dev/null || true
rm -f "/lib/modules/$KVER/updates/ath9k_htc.ko"
rm -f "/lib/modules/$KVER/updates/ath9k_common.ko"
depmod -a "$KVER"

echo "== removing custom dev firmware =="
rm -f "/lib/firmware/ath9k_htc/htc_9271-1.dev.0.fw"

echo "== reloading stock module + firmware =="
modprobe ath9k_htc
sleep 2
dmesg | grep -iE "ath9k_htc" | tail -6

echo "== ROLLBACK DONE =="
