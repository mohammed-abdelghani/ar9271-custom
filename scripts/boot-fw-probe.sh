#!/usr/bin/env bash
# boot-fw-probe.sh — append the boot-time ath9k_htc state to /var/log/ath9k-boot.log.
# Purpose: prove firmware/param persistence after reboots (called by ath9k-boot-probe.service).
LOG=/var/log/ath9k-boot.log

{
	echo "==================== BOOT PROBE $(date -Is) ===================="
	echo "kernel: $(uname -r)  uptime_boot: $(who -b 2>/dev/null | awk '{print $4}')"
	echo "--- dmesg fw/driver lines ---"
	dmesg | grep -iE "Transferred FW|FW Version|CUSTOM BUILD|ath9k_htc" | head -12
	echo "--- module parameters at boot ---"
	for f in /sys/module/ath9k_htc/parameters/*; do
		[ -r "$f" ] && echo "  $(basename "$f") = $(cat "$f")"
	done
	echo "--- installed firmware files ---"
	ls -la /lib/firmware/ath9k_htc/htc_9271* 2>/dev/null
	echo "--- regulatory ---"
	iw reg get 2>/dev/null | head -2
	echo "--- runtime power state ---"
	for d in /sys/bus/usb/devices/*; do
		if [ "$(cat "$d/idVendor" 2>/dev/null)" = "0cf3" ] && [ "$(cat "$d/idProduct" 2>/dev/null)" = "1006" ]; then
			echo "  usb $d control=$(cat "$d/power/control") status=$(cat "$d/power/runtime_status")"
		fi
	done
	IF=$(find /sys/class/net -mindepth 1 -maxdepth 1 -type d -name 'wlx*' -printf '%f\n' 2>/dev/null | sort | head -1)
	[ -n "$IF" ] && echo "  wifi power_save: $(iw dev "$IF" get power_save 2>/dev/null)"
	echo "==============================================================="
	echo
} >> "$LOG" 2>&1
