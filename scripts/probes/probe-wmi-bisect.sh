#!/usr/bin/env bash
# probe-wmi-bisect.sh — Phase-1 live evidence: WMI reply-path bisect.
#
# Method: one debugfs operation at a time; after EVERY op record:
#   wmi_diag (free, host-local: tx_seq/last_seq/timeouts)
#   ping round trip (real WMI: does the target still reply + target counters)
# This separates three cases per op:
#   A) op OK            -> nothing died
#   B) op FAIL + ping OK-> that command specifically fails (target alive)
#   C) op FAIL + ping FAIL -> channel wedged at this op (or earlier)
#
# RUN WITH: sudo ./probe-wmi-bisect.sh [logfile]
set -u

B="${BUILD_DIR:-$HOME/ath9k-fw-build}"
LOG="${1:-$B/bisect-$(date +%Y%m%d-%H%M%S).log}"
ERRF=/tmp/bisect-err.$$; ERRP=/tmp/bisect-ping.$$; OUT=/tmp/bisect-out.$$
trap 'rm -f "$ERRF" "$ERRP" "$OUT"' EXIT

[ "$(id -u)" -eq 0 ] || { echo "run with sudo: sudo $0"; exit 1; }

IFACE=$(ls /sys/class/net/ | grep '^wlx' | head -1)
if [ -z "$IFACE" ]; then echo "FAIL: no wlx interface"; exit 1; fi
N=$(cat "/sys/class/net/$IFACE/phy80211/index")
D="/sys/kernel/debug/ieee80211/phy$N/ath9k_htc"
if [ ! -d "$D" ]; then echo "FAIL: debugfs dir missing at $D (debugfs mounted? driver loaded?)"; exit 1; fi

say(){ echo "$*" | tee -a "$LOG"; }
snap_diag(){ sudo -n cat "$D/wmi_diag" 2>&1 | sed 's/^/    /' | tee -a "$LOG"; }
ifstate(){ ip link show "$IFACE" | head -1 | sed 's/^/    /' | tee -a "$LOG"; }

ping_rt(){
	local payload=$(( (RANDOM * 32768 + RANDOM) & 0x7fffffff ))
	if ! echo "$payload" > "$D/custom_ping" 2>"$ERRP"; then
		say "    ping: FAIL [$(head -c 120 "$ERRP" | tr '\n' ' ')]"
		return 1
	fi
	say "    ping: OK  [$(head -1 "$D/custom_ping" 2>/dev/null)]"
	return 0
}

read_file1(){
	# $1 = file name; prints PASS/FAIL with first line
	local f="$1"
	if ! timeout 8 cat "$D/$f" >"$OUT" 2>"$ERRF"; then
		say "  READ $f: FAIL [$(head -c 120 "$ERRF" | tr '\n' ' ')]"
		return 1
	fi
	say "  READ $f: OK   [$(head -1 "$OUT" | cut -c1-72)]"
	return 0
}

write_file1(){
	# $1=file $2=payload
	local f="$1" v="$2"
	if ! echo "$v" > "$D/$f" 2>"$ERRF"; then
		say "  WRITE $f=$v: FAIL [$(head -c 120 "$ERRF" | tr '\n' ' ')]"
		return 1
	fi
	say "  WRITE $f=$v: OK"
	return 0
}

echo "=== probe-wmi-bisect $(date '+%F %T') iface=$IFACE phy$N ===" | tee -a "$LOG"
say "-- interface state:"; ifstate

# ---- isolation from NetworkManager (user rule: exclude scan interference) --
nmcli device set "$IFACE" managed no 2>/dev/null && say "-- NM unmanaged: yes" || say "-- NM unmanaged: FAILED (continuing, NM may interfere)"
ip link set "$IFACE" down 2>/dev/null && say "-- iface down: yes"
sleep 1

say ""
say "== STEP A: baseline (no WMI op yet) =="
snap_diag; ping_rt

say ""
say "== STEP B: read-only bisect (test-all sections 3..15 order) =="
declare -a FILES=(rate_usage rssi_hist ani_auto macfilter txpower nf rc_fixed rc_profile baw ack_timeout netflow chan_survey auto_spectral blackbox firstep thermal telemetry)
DEAD=0
for f in "${FILES[@]}"; do
	if read_file1 "$f"; then :; else DEAD=$((DEAD+1)); fi
	# after every op: local diag + real ping probe
	snap_diag
	ping_rt || DEAD=$((DEAD+1))
done

say ""
say "== STEP C: sentinel trap (nf/firstep after any wedge) =="
say "-- nf and firstep print values even when REG_READ fails silently;"
say "-- compare with ping result above: value+dead ping = sentinel, not data."
read_file1 nf && read_file1 firstep

say ""
say "== STEP D: write ops for files whose READ passed (sections 6,8-10,13-15) =="
for spec in "macfilter add 0 aa:bb:cc:dd:ee:ff" "baw 32" "ack_timeout 200" "rc_profile 1" "auto_spectral on" "blackbox 0" "firstep 1"; do
	set -- $spec; f=$1; v=$2
	[ $# -ge 3 ] && v="$2 $3"
	if write_file1 "$f" "$v"; then :; else DEAD=$((DEAD+1)); fi
	snap_diag; ping_rt || DEAD=$((DEAD+1))
done

say ""
say "== RESULT: wedged_steps=$DEAD  (see $LOG) =="
say "-- restore NM management:"
nmcli device set "$IFACE" managed yes 2>/dev/null && say "   managed: yes"
echo "$LOG" > /tmp/bisect-last-log
