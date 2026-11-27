#!/usr/bin/env bash
# soak-2h.sh — 2-hour stability soak on the FINAL configuration.
# Logs to soak-run2.log: per-10s ping, per-5min load/metrics blocks, final summary.
# Watchdog (ath9k-watchdog.service) runs alongside; its triggers are counted here.
set -u
cd "$(dirname "$0")" || exit 1
LOG=soak-run2.log
IFACE=$(find /sys/class/net -mindepth 1 -maxdepth 1 -type d -name 'wlx*' -printf '%f\n' 2>/dev/null | sort | head -1)
[ -n "$IFACE" ] || { echo "no wlx* interface found"; exit 1; }
# USB device node for power/control — resolved via the bound driver, not a fixed path
USBIF=$(basename "$(ls -d /sys/bus/usb/drivers/ath9k_htc/*:* 2>/dev/null | head -1)")
USBDEV=${USBIF%%:*}
GW=""   # auto-discovered each check (subnet can change)
DUR=7200
STEP=10
# auto-detect the ath9k_htc debugfs dir (phy index changes across reloads/reboots)
D=$(ls -d /sys/kernel/debug/ieee80211/phy*/ath9k_htc 2>/dev/null | head -1)

{
echo "==================== SOAK START $(date -Is) ===================="
echo "config snapshot:"
for f in /sys/module/ath9k_htc/parameters/*; do echo "  $(basename $f)=$(cat $f)"; done
sudo -n /usr/bin/dmesg | grep -E "Transferred FW|FW Version" | tail -2 | sed 's/^/  /'
echo "  regdom: $(sudo -n /usr/sbin/iw reg get | head -1)"
echo "  usb:    $(cat /sys/bus/usb/devices/$USBDEV/power/control 2>/dev/null) $(cat /sys/bus/usb/devices/$USBDEV/power/runtime_status 2>/dev/null)"
echo "  psave:  $(sudo -n /usr/sbin/iw dev $IFACE get power_save)"
echo "  dbgmsk: $(sudo -n /usr/bin/cat $D/debug 2>/dev/null)"
echo "  wd:     $(systemctl is-active ath9k-watchdog.service)"
} >> "$LOG" 2>&1

N=0; LOST=0; SUM=0; MAX=0; SPIKES=0; STREAK=0; MAXSTREAK=0
DMARK=$(sudo -n /usr/bin/dmesg | wc -l)

while [ $((N * STEP)) -lt $DUR ]; do
	GW=$(ip route show dev $IFACE 2>/dev/null | awk '/^default/ {print $3; exit}')
	if [ -z "$GW" ]; then echo "$(date +%H:%M:%S) no GW via iface (DHCP pending)" >> "$LOG"; sleep $STEP; continue; fi
	OUT=$(ping -I $IFACE -c1 -W2 $GW 2>&1)
	TS=$(date +%H:%M:%S)
	if echo "$OUT" | grep -q "time="; then
		RTT=$(echo "$OUT" | grep -o 'time=[0-9.]*' | cut -d= -f2)
		STREAK=0
		SUM=$(awk "BEGIN{print $SUM+$RTT}")
		RES=$(awk "BEGIN{print ($RTT>MAX)?1:0}")
		[ "$RES" = "1" ] && MAX=$RTT
		RES=$(awk "BEGIN{print ($RTT>50)?1:0}")
		[ "$RES" = "1" ] && SPIKES=$((SPIKES+1)) && echo "$TS ping SPIKE $RTT ms" >> "$LOG"
	else
		LOST=$((LOST+1)); STREAK=$((STREAK+1))
		[ "$STREAK" -gt "$MAXSTREAK" ] && MAXSTREAK=$STREAK
		echo "$TS ping FAIL (streak=$STREAK)" >> "$LOG"
	fi
	N=$((N+1))

	if [ $((N % 30)) -eq 0 ]; then
		{
		echo "---- 5-min block $((N/30)) t=$((N*STEP))s $(date -Is) ----"
		sudo -n /usr/sbin/iw dev $IFACE station dump | grep -E "tx retries|tx failed|beacon loss|tx bitrate|signal:" | tr '\n' ' ' | sed 's/^/  st: /'; echo
		IFIP=$(ip -4 addr show dev $IFACE 2>/dev/null | grep -o 'inet [0-9.]*' | awk '{print $2}' | head -1)
		SPD=$(curl --interface "$IFIP" -sS -o /dev/null -w '%{speed_download}' --max-time 90 http://cachefly.cachefly.net/10mb.test 2>/dev/null)
		echo "  dl: ${SPD:-0} B/s"
		echo "  usb: $(cat /sys/bus/usb/devices/$USBDEV/power/control 2>/dev/null)/$(cat /sys/bus/usb/devices/$USBDEV/power/runtime_status 2>/dev/null)  wd: $(systemctl is-active ath9k-watchdog.service)"
		NEW=$(sudo -n /usr/bin/dmesg | tail -n +$((DMARK+1)) | grep -icE "timeout|fail|reset|disconnect|wedge")
		echo "  dmesg new error-lines since start: $NEW"
		DMARK=$(sudo -n /usr/bin/dmesg | wc -l)
		grep -c "RECOVERY TRIGGER" /var/log/ath9k-watchdog.log 2>/dev/null | sed 's/^/  wd triggers so far: /'
		} >> "$LOG" 2>&1
	fi
	sleep $STEP
done

{
echo "==================== SOAK END $(date -Is) ===================="
echo "samples=$N lost=$LOST loss_pct=$(awk "BEGIN{printf \"%.2f\", $LOST*100/$N}")%"
echo "rtt avg=$(awk "BEGIN{if($N>$LOST) printf \"%.1f\", $SUM/($N-$LOST)}")  max=$MAX ms  spikes>50ms=$SPIKES  max_fail_streak=$MAXSTREAK"
echo "watchdog RECOVERY TRIGGER count: $(grep -c 'RECOVERY TRIGGER' /var/log/ath9k-watchdog.log 2>/dev/null)"
echo "watchdog log tail:"; tail -5 /var/log/ath9k-watchdog.log 2>/dev/null | sed 's/^/  /'
} >> "$LOG" 2>&1
echo "SOAK DONE"
