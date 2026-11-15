#!/usr/bin/env bash
# ath9k-watchdog.sh — hang watchdog for the AR9271 USB adapter (ath9k_htc).
#
# Detection (the hang signature from docs/13):
#   * definite stall : gateway ping fails STALL_LIMIT consecutive checks AND
#                      tx_packets frozen (URBs never complete) -> recover
#   * soft stall     : gateway ping fails SOFT_LIMIT checks AND rx_packets frozen
#                      (beacons gone too) -> recover
#   * ping fails but counters keep moving -> association/routing problem, NOT a
#                      driver hang: log it, do not reset (NM handles re-associate)
#   * interface missing -> someone is doing a driver reload: log, wait, no action
#
# Recovery ladder (per docs/13 golden rules):
#   1. driver unbind + bind        (graceful, keeps USB enumeration)
#   2. usbreset (USB-level reset)  (re-enumerates; firmware reboots)
#   3. after MAX_RECOVERIES in the rate-limit window: log only + advise physical replug
#
# Modes:
#   --loop       daemon mode (used by ath9k-watchdog.service)
#   --once       single check, exit 0
#   --selftest   inject static counters + unreachable target, DRYRUN: no sysfs writes
#   --install    write + enable the systemd unit
#
# Run: sudo ./ath9k-watchdog.sh --install   (then it loops as the systemd unit)
set -uo pipefail

IFACE=wlxf8d111b37c15
VID=0cf3 PID=1006
LOG=/var/log/ath9k-watchdog.log
CHECK_INTERVAL=5          # seconds between checks
STALL_LIMIT=3             # consecutive definite-stall checks before recovery (~15-20s)
SOFT_LIMIT=12             # consecutive soft checks before recovery (~60s)
MAX_RECOVERIES=3          # max recoveries per RATE_WINDOW seconds
RATE_WINDOW=3600
GW=""                     # discovered from the interface route

MODE=${1:-}
DRYRUN=0
STALL_N=0
SOFT_N=0
MISSING_N=0
LAST_TX=-1 LAST_RX=-1
EPISODE_ALERTED=0
MISSING_GW_LOGGED=0

log() { printf '%s %s\n' "$(date -Is)" "$*" | tee -a "$LOG" >&2; }

# ---- injected sources (selftest overrides) --------------------------------
read_counter() { # $1=iface $2=tx_packets|rx_packets -> value
	if [ "$MODE" = "--selftest" ]; then cat "$STALL_INJ_FILE"; else cat "/sys/class/net/$1/statistics/$2"; fi
}
do_ping() {
	if [ "$MODE" = "--selftest" ]; then ping -c1 -W1 -I lo 192.0.2.1 >/dev/null 2>&1; else ping -c1 -W2 -I "$1" "$GW" >/dev/null 2>&1; fi
}
sysfs_write() { # $1=path $2=value
	if [ "$DRYRUN" = "1" ]; then log "DRYRUN: echo '$2' > $1"; else echo "$2" > "$1" 2>&1 || log "ERROR writing '$2' to $1"; fi
}

# ---- recovery --------------------------------------------------------------
recover() { # $1=reason
	log "RECOVERY TRIGGER: $1 (stall_n=$STALL_N soft_n=$SOFT_N)"
	# rate limiting
	NOW=$(date +%s); REC_N=0
	[ -f /run/ath9k-watchdog.rec ] && while read -r t; do
		[ $((NOW - t)) -lt $RATE_WINDOW ] && REC_N=$((REC_N + 1))
	done < /run/ath9k-watchdog.rec
	if [ "$REC_N" -ge "$MAX_RECOVERIES" ]; then
		# v4.2: log the refusal ONCE per stall episode (the live 2026-11-15
		# event spammed 3 lines every 5 s for minutes — log noise, no info)
		if [ "${EPISODE_ALERTED:-0}" != "1" ]; then
			log "ALERT: $REC_N recoveries in the last ${RATE_WINDOW}s — not recovering again this hour (logging once per episode)."
			log "ALERT: if the link stays dead, PHYSICAL REPLUG is required (docs/13 golden rule)."
			EPISODE_ALERTED=1
		fi
		return
	fi

	# 1) graceful: driver unbind/bind
	IFACEDIR=$(readlink -f "/sys/class/net/$IFACE/device" 2>/dev/null)
	IFACEDEV=$(basename "${IFACEDIR:-}")
	if [ -z "$IFACEDEV" ]; then log "no device dir for $IFACE — skipping unbind"; else
		log "recovery step 1: unbind $IFACEDEV from ath9k_htc"
		sysfs_write /sys/bus/usb/drivers/ath9k_htc/unbind "$IFACEDEV"
		sleep 2
		sysfs_write /sys/bus/usb/drivers/ath9k_htc/bind "$IFACEDEV"
		for i in $(seq 1 8); do
			[ -d "/sys/class/net/$IFACE" ] && break; sleep 1
		done
	fi
	if [ -d "/sys/class/net/$IFACE" ] && do_ping "$IFACE"; then
		log "RECOVERY OK (unbind/bind): $IFACE back, gateway answers"
		echo "$NOW" >> /run/ath9k-watchdog.rec; STALL_N=0; SOFT_N=0; EPISODE_ALERTED=0; return
	fi

	# 2) userspace re-establishment: NM reconnect re-runs association + DHCP.
	# v4.2 (live case 2026-11-15): the driver came back after unbind/bind but
	# ping kept failing — the iface had no valid IP (new DHCP subnet after an
	# AP reconfig). A NM reconnect fixes the userspace half without touching USB.
	log "recovery step 2: NM reconnect (re-association + DHCP)"
	if [ "$DRYRUN" = "1" ]; then log "DRYRUN: nmcli reconnect skipped"; else
		nmcli device disconnect "$IFACE" >/dev/null 2>&1 || true
		sleep 2
		nmcli device connect "$IFACE" >/dev/null 2>&1 || true
	fi
	for i in $(seq 1 20); do
		do_ping "$IFACE" && break; sleep 1
	done
	if do_ping "$IFACE"; then
		log "RECOVERY OK (NM reconnect): $IFACE answering"
		echo "$NOW" >> /run/ath9k-watchdog.rec; STALL_N=0; SOFT_N=0; EPISODE_ALERTED=0; return
	fi

	# 3) USB-level reset (device re-enumerates, firmware reboots)
	BUSNUM=$(cat "/sys/bus/usb/devices/${IFACEDEV%%:*}/busnum" 2>/dev/null || true)
	DEVNUM=$(cat "/sys/bus/usb/devices/${IFACEDEV%%:*}/devnum" 2>/dev/null || true)
	if [ -n "$BUSNUM" ] && [ -n "$DEVNUM" ]; then
		log "recovery step 3: usbreset /dev/bus/usb/$(printf '%03d' "$BUSNUM")/$(printf '%03d' "$DEVNUM")"
		if [ "$DRYRUN" = "1" ]; then log "DRYRUN: usbreset skipped"; else /usr/bin/usbreset "/dev/bus/usb/$(printf '%03d' "$BUSNUM")/$(printf '%03d' "$DEVNUM")" >> "$LOG" 2>&1 || log "usbreset returned rc=$?"; fi
		for i in $(seq 1 30); do
			[ -d "/sys/class/net/$IFACE" ] && sleep 2 && break; sleep 1
		done
	else
		log "recovery step 3: cannot resolve busnum/devnum — device may be gone from USB"
	fi
	if [ -d "/sys/class/net/$IFACE" ] && do_ping "$IFACE"; then
		log "RECOVERY OK (usbreset): $IFACE back, gateway answers"
		EPISODE_ALERTED=0
	else
		# after a USB reset NM usually re-probes; give DHCP a window, then judge
		sleep 10
		if [ "$DRYRUN" != "1" ]; then nmcli device connect "$IFACE" >/dev/null 2>&1 || true; fi
		for i in $(seq 1 15); do
			do_ping "$IFACE" && break; sleep 1
		done
		if do_ping "$IFACE"; then
			log "RECOVERY OK (usbreset + NM reconnect): $IFACE answering"
			EPISODE_ALERTED=0
		else
			log "RECOVERY FAILED: $IFACE still not answering after unbind/bind + NM reconnect + usbreset"
			log "ADVICE: physical replug needed (docs/13: full wedge can require replug)"
		fi
	fi
	echo "$NOW" >> /run/ath9k-watchdog.rec
	STALL_N=0; SOFT_N=0
}

# ---- one check -------------------------------------------------------------
check_once() {
	# v4.2: re-derive the gateway EVERY check — the DHCP subnet can change
	# mid-run (live case 2026-11-15: 10.64.17.158 -> 172.17.235.167 after an
	# AP reconfig); a cached target turned a healthy link into "permanent fail".
	GW=$(ip route show dev "$IFACE" 2>/dev/null | awk '/^default/ {print $3; exit}')
	if [ "$MODE" = "--selftest" ]; then GW=192.0.2.1; fi

	if [ ! -d "/sys/class/net/$IFACE" ]; then
		MISSING_N=$((MISSING_N + 1))
		[ "$MISSING_N" -eq 1 ] && log "iface $IFACE missing — waiting (driver reload or replug in progress)"
		STALL_N=0; SOFT_N=0; LAST_TX=-1; LAST_RX=-1; EPISODE_ALERTED=0
		return 0
	fi
	[ "$MISSING_N" -gt 0 ] && { log "iface $IFACE back"; MISSING_N=0; }

	# no route via the iface = association/DHCP not (yet) established: that is
	# an NM/userspace state, not a driver hang — wait it out, never reset for it
	if [ -z "$GW" ]; then
		[ "${MISSING_GW_LOGGED:-0}" != "1" ] && { log "no default route via $IFACE yet (association/DHCP pending) — waiting"; MISSING_GW_LOGGED=1; }
		STALL_N=0; SOFT_N=0; LAST_TX=-1; LAST_RX=-1; EPISODE_ALERTED=0
		return 0
	fi
	MISSING_GW_LOGGED=0

	TX=$(read_counter "$IFACE" tx_packets) || TX=0
	RX=$(read_counter "$IFACE" rx_packets) || RX=0
	[ -z "$TX" ] && TX=0; [ -z "$RX" ] && RX=0

	if do_ping "$IFACE"; then
		PING_OK=1
	else
		PING_OK=0
	fi

	if [ "$PING_OK" -eq 1 ]; then
		[ "$STALL_N" -gt 0 ] || [ "$SOFT_N" -gt 0 ] && log "link recovered by itself (no action taken): stall_n=$STALL_N soft_n=$SOFT_N"
		STALL_N=0; SOFT_N=0; LAST_TX=$TX; LAST_RX=$RX; EPISODE_ALERTED=0
		return 0
	fi

	TX_MOVED=0; RX_MOVED=0
	[ "$LAST_TX" -ge 0 ] && [ "$TX" -ne "$LAST_TX" ] && TX_MOVED=1
	[ "$LAST_RX" -ge 0 ] && [ "$RX" -ne "$LAST_RX" ] && RX_MOVED=1

	if [ "$TX_MOVED" -eq 1 ] || [ "$RX_MOVED" -eq 1 ]; then
		# ping fails but the driver is still moving frames: link/association problem
		STALL_N=0; SOFT_N=$((SOFT_N + 1))
		if [ "$SOFT_N" -eq 1 ] || [ $((SOFT_N % 6)) -eq 0 ]; then
			log "ping FAIL but counters moving (tx=$TX rx=$RX) — association/link problem, not a driver hang. No reset (count ${SOFT_N}/${SOFT_LIMIT})."
		fi
		LAST_TX=$TX; LAST_RX=$RX
		return 0
	fi

	# counters frozen while ping fails -> real stall
	STALL_N=$((STALL_N + 1)); SOFT_N=$((SOFT_N + 1))
	# v4.2: log the stall line once per 4 checks (long stalls spam otherwise);
	# recovery-trigger lines always log
	if [ $((STALL_N % 4)) -eq 1 ]; then
		log "STALL ${STALL_N} (limit ${STALL_LIMIT}): ping fail + counters frozen (tx=$TX rx=$RX)"
	fi
	if [ "$STALL_N" -ge "$STALL_LIMIT" ]; then
		recover "definite stall (tx frozen + ping fail x$STALL_N)"
	elif [ "$SOFT_N" -ge "$SOFT_LIMIT" ]; then
		recover "soft stall (ping fail x$SOFT_N + counters frozen)"
	fi
	LAST_TX=$TX; LAST_RX=$RX
}

# ---- selftest --------------------------------------------------------------
selftest() {
	DRYRUN=1
	STALL_INJ=$(mktemp)
	STALL_INJ_FILE="$STALL_INJ"
	echo 1000 > "$STALL_INJ"   # static counter: never moves -> guaranteed stall
	log "SELFTEST start: mode=dryrun iface=fake(lo counters) target=192.0.2.1"
	# exercise: 3 consecutive stalls -> unbind/bind dry-run -> usbreset dry-run
	STALL_LIMIT=2 SOFT_LIMIT=4 CHECK_INTERVAL=0
	IFACE=lo
	log "-- phase A: definite stall ladder --"
	for i in 1 2 3; do check_once; done
	log "-- phase B: counters moving (association problem) -> NO reset --"
	echo 1050 > "$STALL_INJ"
	STALL_N=0; SOFT_N=0; LAST_TX=1049; LAST_RX=5000
	check_once
	log "-- phase C: counters static again -> recovery resumes --"
	echo 1050 > "$STALL_INJ"; LAST_TX=1050; LAST_RX=5000
	for i in 1 2; do check_once; done
	log "-- phase D: rate limit: after MAX_RECOVERIES the next trigger must be refused --"
	NOW=$(date +%s)
	for i in 1 2 3; do echo "$NOW" >> /run/ath9k-watchdog.rec.test; done
	cp /run/ath9k-watchdog.rec.test /run/ath9k-watchdog.rec 2>/dev/null
	recover "selftest rate-limit check"
	rm -f /run/ath9k-watchdog.rec /run/ath9k-watchdog.rec.test "$STALL_INJ"
	log "SELFTEST end"
}

# ---- install ---------------------------------------------------------------
install_unit() {
	[ "$(id -u)" -eq 0 ] || { echo "run with sudo"; exit 2; }
	cat > /etc/systemd/system/ath9k-watchdog.service <<UNIT
[Unit]
Description=ath9k_htc USB hang watchdog (tx/rx stall + ping loss detection, unbind/bind + usbreset recovery)
After=NetworkManager.service
Wants=NetworkManager.service

[Service]
Type=simple
ExecStart=$(readlink -f "$0") --loop
Restart=always
RestartSec=10

[Install]
WantedBy=multi-user.target
UNIT
	systemctl daemon-reload
	systemctl enable --now ath9k-watchdog.service
	systemctl --no-pager status ath9k-watchdog.service | head -8
}

# ---- main ------------------------------------------------------------------
# NOTE: the flock is taken only by --loop/--once (an --install invocation must
# not hold it, or the service it just started would exit immediately).
case "$MODE" in
	--selftest) selftest ;;
	--install) install_unit ;;
	--once|--loop)
		exec 9>/run/ath9k-watchdog.lock
		flock -n 9 || { echo "another instance is running"; exit 0; }
esac

case "$MODE" in
	--selftest) exit 0 ;;
	--install) exit 0 ;;
	--once) check_once ;;
	--loop)
		log "watchdog started (iface=$IFACE interval=${CHECK_INTERVAL}s stall_limit=$STALL_LIMIT soft_limit=$SOFT_LIMIT)"
		trap 'log "watchdog stopped"; exit 0' INT TERM
		while true; do check_once; sleep "$CHECK_INTERVAL"; done
		;;
	*) echo "usage: $0 --loop|--once|--selftest|--install"; exit 2 ;;
esac
