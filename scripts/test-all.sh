#!/usr/bin/env bash
# test-all.sh — comprehensive verification of the CUSTOM BUILD (v3+v4)
#
# CUSTOM BUILD v4-fix (Phase 0):
#   * requires root (debugfs + WMI need it) — no more silent empty reads
#   * D is computed with root privileges and verified non-empty, else FATAL
#   * every debugfs write goes through `tee` with an exit-code check
#   * every read that errors (timeout/permission) = explicit FAIL
#   * WMI health check (custom ping) before the suite and between sections:
#     the first broken section is identified instead of 13 silent timeouts
#   * final PASS/FAIL summary + meaningful exit code
#   * runs with the interface UNMANAGED (NM interference excluded), restores
#     NM at the end
# RUN WITH: sudo ./test-all.sh
set -u

IFACE=$(ls /sys/class/net/ | grep '^wlx' | head -1)
if [ -z "$IFACE" ]; then
	echo "FATAL: interface missing — replug/reload first"; exit 2
fi
[ "$(id -u)" -eq 0 ] || { echo "FATAL: run with sudo — debugfs/WMI need root: sudo $0"; exit 2; }

N=$(cat "/sys/class/net/$IFACE/phy80211/index")
D=$(ls -d /sys/kernel/debug/ieee80211/phy*/ath9k_htc 2>/dev/null | head -1)
if [ -z "$D" ] || ! [ -f "$D/wmi_diag" ]; then
	echo "FATAL: ath9k_htc debugfs dir not found (computed: '$D') — debugfs mounted and driver loaded?"
	exit 2
fi

PASS=0; FAIL=0; SECT_FAIL=0
declare -a FAILED_STEPS
ok()  { PASS=$((PASS+1)); echo "  PASS: $*"; }
bad() { FAIL=$((FAIL+1)); FAILED_STEPS+=("$*"); echo "  FAIL: $*"; }

# readf <file> [label] — one debugfs read; timeout/error = FAIL
readf() {
	local f=$1 out
	if ! out=$(timeout 10 cat "$D/$f" 2>&1); then
		bad "$f read: $(echo "$out" | head -1)"
		return 1
	fi
	ok "$f read OK"
	echo "$out" | sed 's/^/    | /'
	return 0
}

# writef <file> <value> — one debugfs write through tee; error = FAIL
writef() {
	local f=$1 v=$2 out
	if ! out=$(printf '%s\n' "$v" | timeout 10 tee "$D/$f" 2>&1 >/dev/null); then
		bad "$f write '$v': ${out:-write error}"
		return 1
	fi
	ok "$f write '$v' accepted"
	return 0
}

# health <label> — WMI control path must answer; else abort the suite
# v4.1: counts go through sanity_count() so impossible numbers can never be
# reported silently (the old "46K timeouts" artifact class).
SANITY_BAD=0
sanity_count() { # sanity_count <name> <value> — echo value, set SANITY_BAD=1 if impossible
	local name=$1 v=$2
	case "$v" in
		''|*[!0-9]*)
			echo "sanity: $name unparseable ('$v')" >&2; SANITY_BAD=1; echo 0; return 1 ;;
	esac
	echo "$v"; return 0
}
health() {
	local tag="health[$1]"
	local before after pong pload tx rsp
	SANITY_BAD=0
	# v4-fix: the diag is multi-line now — take the FIRST token of the
	# cmd_timeouts field only (the old awk $2 pulled rsp_count along and
	# displayed phantom "46060 rsp_count=49460" timeout counts).
	before=$(grep -o 'cmd_timeouts=[0-9]*' "$D/wmi_diag" 2>/dev/null | head -1 | cut -d= -f2)
	if ! timeout 10 bash -c "echo $((RANDOM)) > '$D/custom_ping'" 2>/dev/null; then
		bad "$tag: WMI channel dead (ping timeout) — aborting remaining sections"
		return 1
	fi
	after=$(grep -o 'cmd_timeouts=[0-9]*' "$D/wmi_diag" 2>/dev/null | head -1 | cut -d= -f2)
	pong=$(grep -o 'last_pong=[0-9]*' "$D/custom_ping" | head -1 | cut -d= -f2)
	pload=$(grep -o 'last_payload=0x[0-9a-f]*' "$D/custom_ping" | head -1 | cut -d= -f2)

	# --- sanity gate: refuse to report impossible numbers -----------------
	before=$(sanity_count "before" "$before")
	after=$(sanity_count "after" "$after")
	if [ "$SANITY_BAD" -eq 1 ]; then
		bad "$tag: wmi_diag counters unparseable — diag format changed or read failed"
		return 1
	fi
	# wrap-aware u16 delta (doc: "issued−timeouts=replies across the u16 wrap")
	local d=$(( (after - before + 65536) % 65536 ))
	# a health window spans at most a handful of commands; a larger jump is
	# impossible and means a parsing/diag artifact, not real timeouts
	if [ "$d" -gt 6 ]; then
		bad "$tag: IMPOSSIBLE cmd_timeouts jump $before->$after (delta=$d) — parser/diag artifact, not reporting as real"
		return 1
	fi
	if [ "$d" -gt 0 ]; then
		# ping answered, yet the timeout counter moved: verify against the
		# diag's own invariant (issued - timeouts == replies) before believing it
		tx=$(grep -o 'tx_seq_id=[0-9]*' "$D/wmi_diag" | head -1 | cut -d= -f2)
		rsp=$(grep -o 'rsp_count=[0-9]*' "$D/wmi_diag" | head -1 | cut -d= -f2)
		if [ -n "$tx" ] && [ -n "$rsp" ] && [ $(( (tx - after + 65536) % 65536 )) -ne $(( rsp % 65536 )) ]; then
			bad "$tag: diag inconsistent (tx=$tx timeouts=$after rsp=$rsp do not reconcile) — artifact suspected"
			return 1
		fi
		bad "$tag: ping OK but cmd_timeouts moved $before->$after (real timeout inside the window)"
		return 1
	fi
	ok "$tag: ping OK pong=$pong payload=$pload cmd_timeouts=$before->$after"
	return 0
}

sect_hdr() { SECT_FAIL=0; echo; echo "== $* =="; }
sect_end() {
	if [ "$SECT_FAIL" -eq 0 ]; then echo "  -- section OK"
	else echo "  -- section FAILED ($SECT_FAIL step(s))"; fi
}

echo "=================================================================="
echo " CUSTOM BUILD v4 — full verification ($IFACE, phy$N)"
echo " D=$D"
echo "=================================================================="

# isolation: keep NM away for the whole suite, iface UP (radio on, no scans)
nmcli device set "$IFACE" managed no 2>/dev/null && echo "-- NM unmanaged: yes" || echo "-- NM unmanaged: FAILED (results may be polluted)"
ip link set "$IFACE" up 2>/dev/null && echo "-- iface up (radio on, unmanaged)" || true
# PS off: with mac80211 power-save the target cycles full-sleep/wake and the
# ROM re-shadows MAC registers on every wake (proven live: an ack_timeout
# write reverted to the pre-write value; with PS off it persists).
iw dev "$IFACE" set power_save off 2>/dev/null && echo "-- power_save off: yes" || echo "-- power_save off: FAILED (register knobs may read stale values)"
# ONE deliberate scan: the radio must visit channels at least once for the
# target's NF calibration and the chan_survey table to hold real samples
# (proven live: an idle unassociated radio reports un-calibrated NF garbage).
iw dev "$IFACE" scan trigger >/dev/null 2>&1 && echo "-- scan trigger: done (NF/chan_survey sampling)" || echo "-- scan trigger: skipped"
sleep 5

echo
echo "== 0) WMI health baseline =="
health "baseline" || { echo; echo "== SUITE ABORTED at baseline =="; exit 1; }

echo
echo "== 1) custom WMI ping round trip (echo==payload, pong++) =="
PONG0=$(awk -F'last_pong=' '{print $2}' "$D/custom_ping" | awk '{print $1}')
if writef custom_ping 777; then
	sleep 0.2
	OUT=$(timeout 5 cat "$D/custom_ping" 2>&1) || bad "custom_ping read"
	PONG1=$(echo "$OUT" | grep -o 'last_pong=[0-9]*' | head -1 | cut -d= -f2)
	ECHO=$(echo "$OUT" | grep -o 'last_payload=0x[0-9a-f]*' | head -1 | cut -d= -f2)
	ALLOCF=$(echo "$OUT" | grep -o 'rsp_allocfail=[0-9]*' | head -1 | cut -d= -f2)
	if [ "$ECHO" = "0x00000309" ] && [ "$PONG1" -gt "$PONG0" ] 2>/dev/null && [ "$ALLOCF" = "0" ]; then
		ok "ping echo=0x00000309 pong=$PONG0->$PONG1 allocfail=0"
		echo "$OUT" | sed 's/^/    | /'
	else
		bad "ping mismatch (echo=$ECHO pong=$PONG0->$PONG1 allocfail=$ALLOCF)"
		SECT_FAIL=$((SECT_FAIL+1))
	fi
else
	SECT_FAIL=$((SECT_FAIL+1))
fi
health "after-section-1" || { echo; echo "== SUITE ABORTED =="; exit 1; }

echo
echo "== 2) telemetry (periodic events withdrawn by design — cached) =="
if OUT=$(timeout 5 cat "$D/telemetry" 2>&1); then
	SEQ=$(echo "$OUT" | awk -F'seq=' '{print $2}' | awk '{print $1}')
	SAMPLES=$(echo "$OUT" | awk -F'samples=' '{print $2}' | awk '{print $1}')
	ok "telemetry read OK (seq=$SEQ samples=$SAMPLES)"
	if [ "$SEQ" = "0" ] && [ "$SAMPLES" = "0" ]; then
		echo "    | NOTE: seq=0 samples=0 — NO events received: these are CACHED"
		echo "    | zeros, not live data (periodic telemetry is withdrawn)."
	fi
	echo "$OUT" | sed 's/^/    | /'
else
	bad "telemetry read"; SECT_FAIL=$((SECT_FAIL+1))
fi

echo
echo "== 3) per-rate TX usage (live WMI round trip) =="
readf rate_usage || SECT_FAIL=$((SECT_FAIL+1))
echo "    (populates with TX traffic; write 1 to reset)"

echo
echo "== 4) RSSI histogram (live, 48 bins — the former 64B-boundary killer) =="
readf rssi_hist || SECT_FAIL=$((SECT_FAIL+1))
health "after-rssi_hist(60B-reply)" || { echo; echo "== SUITE ABORTED: 60B reply still wedges the device =="; exit 1; }

echo
echo "== 5) adaptive ANI =="
readf ani_auto || SECT_FAIL=$((SECT_FAIL+1))

echo
echo "== 6) MAC filter (add -> present -> clear -> empty -> mode 0) =="
if writef macfilter "add 0 aa:bb:cc:dd:ee:ff"; then
	OUT=$(timeout 5 cat "$D/macfilter" 2>&1) && { ok "macfilter read"; echo "$OUT" | sed 's/^/    | /'
		echo "$OUT" | grep -qi "aa:bb:cc:dd:ee:ff" && ok "mac aa:bb:cc:dd:ee:ff present" || { bad "mac NOT in list"; SECT_FAIL=$((SECT_FAIL+1)); }
	} || { bad "macfilter read"; SECT_FAIL=$((SECT_FAIL+1)); }
	writef macfilter clear || SECT_FAIL=$((SECT_FAIL+1))
	writef macfilter "mode 0"  || SECT_FAIL=$((SECT_FAIL+1))
	readf macfilter >/dev/null && true || SECT_FAIL=$((SECT_FAIL+1))
else
	SECT_FAIL=$((SECT_FAIL+1))
fi

echo
echo "== 7) TX power / NF / RC knobs =="
readf txpower   || SECT_FAIL=$((SECT_FAIL+1))
# v4.1 NF honesty gate (Phase 3.3): the nf node reads the AR_PHY_CCA MINCCA
# field with the CORRECT AR9002 mask (driver fix v4.1: shift 20, raw= printed).
# The AR9271's vendor limits (ar9002_phy.h): nominal -118, good -127..-116.
#  [-127..-116]  GOOD (vendor window for the 9271)
#  (-116..-60]   plausible-but-elevated (live in-channel energy; cross-check)
#  (-60..-10]    far too high — CCA/RSSI-suspected, do NOT report as NF
#  > -10 / < -130 / unparseable — un-calibrated/implausible
if OUT=$(timeout 5 cat "$D/nf" 2>&1); then
	NFVAL=$(echo "$OUT" | grep -o 'noise_floor=[-0-9]*' | head -1 | cut -d= -f2)
	ASSOC=$(iw dev "$IFACE" link 2>/dev/null | grep -c "Connected to")
	if ! [[ "$NFVAL" =~ ^-[0-9]+$ ]]; then
		bad "nf read unparseable ('${NFVAL:-empty}')"; SECT_FAIL=$((SECT_FAIL+1))
	elif [ "$NFVAL" -gt -10 ] || [ "$NFVAL" -le -130 ]; then
		bad "nf=$NFVAL dBm — un-calibrated/implausible; radio is$([ "$ASSOC" -eq 0 ] && echo ' UNASSOCIATED (idle)' || echo ' associated but un-calibrated') — NOT a real noise floor"
		SECT_FAIL=$((SECT_FAIL+1))
	elif [ "$NFVAL" -gt -60 ]; then
		bad "nf=$NFVAL dBm — TOO HIGH for a real noise floor: CCA/RSSI-suspected register, do NOT report as NF"
		SECT_FAIL=$((SECT_FAIL+1))
	elif [ "$NFVAL" -gt -116 ]; then
		echo "  WARN: nf=$NFVAL dBm — above the 9271 good window (-127..-116): elevated in-channel energy or stale calibration. Cross-check."
	else
		ok "nf=$NFVAL dBm (in 9271 good window -127..-116)"
	fi
	if [ "$ASSOC" -eq 0 ]; then
		echo "    | NOTE: radio is UNASSOCIATED in this suite — value is the last calibrated NF (stale), not a fresh sample"
	fi
	echo "$OUT" | sed 's/^/    | /'
else
	bad "nf read"; SECT_FAIL=$((SECT_FAIL+1))
fi
readf rc_fixed   || SECT_FAIL=$((SECT_FAIL+1))
readf rc_profile || SECT_FAIL=$((SECT_FAIL+1))

echo
echo "== 8) v3: Block-ACK window runtime (round trip 32 -> 64 -> 64) =="
if writef baw 32; then
	V=$(timeout 5 cat "$D/baw" 2>&1) || V="READ-FAIL"
	echo "$V" | grep -q "32" && ok "baw=32 applied" || { bad "baw readback: $V"; SECT_FAIL=$((SECT_FAIL+1)); }
	writef baw 64 || SECT_FAIL=$((SECT_FAIL+1))
	V=$(timeout 5 cat "$D/baw" 2>&1) || V="READ-FAIL"
	echo "$V" | grep -q "64" && ok "baw=64 applied" || { bad "baw readback: $V"; SECT_FAIL=$((SECT_FAIL+1)); }
else
	SECT_FAIL=$((SECT_FAIL+1))
fi

echo
echo "== 9) v3: long-distance ACK timeout (200 -> 32) =="
# KNOWN LIMITATION (docs/10 §6, refined this session): the WMI round trip and
# the target-side write are proven (blackbox logs the received value), but
# register 0x8014 on the K2 does not hold a host-written value: mac80211
# rewrites it on every channel config and the ROM re-shadows MAC registers
# across sleep/wake transitions (values 16111/2816 observed). The readback
# therefore reflects the platform's register traffic, not the knob. FAIL is
# kept on purpose: end-to-end effect of this knob remains UNPROVEN.
if writef ack_timeout 200; then
	V=$(timeout 5 cat "$D/ack_timeout" 2>&1) || V="READ-FAIL"
	echo "$V" | grep -q "200" && ok "ack_timeout=200 applied" || { bad "ack_timeout readback does not hold (platform rewrites 0x8014): $V"; SECT_FAIL=$((SECT_FAIL+1)); }
	writef ack_timeout 32 || SECT_FAIL=$((SECT_FAIL+1))
	V=$(timeout 5 cat "$D/ack_timeout" 2>&1) || V="READ-FAIL"
	echo "$V" | grep -q "32"  && ok "ack_timeout=32 applied" || { bad "ack_timeout readback does not hold (platform rewrites 0x8014): $V"; SECT_FAIL=$((SECT_FAIL+1)); }
else
	SECT_FAIL=$((SECT_FAIL+1))
fi

echo
echo "== 10) v3: RC profile (1=clean-air 2=noisy-air round trip) =="
if writef rc_profile 1; then
	V=$(timeout 5 cat "$D/rc_profile" 2>&1) || V="READ-FAIL"
	echo "$V" | grep -q "1" && ok "rc_profile=1 applied" || { bad "rc_profile readback: $V"; SECT_FAIL=$((SECT_FAIL+1)); }
	writef rc_profile 2 || SECT_FAIL=$((SECT_FAIL+1))
	V=$(timeout 5 cat "$D/rc_profile" 2>&1) || V="READ-FAIL"
	echo "$V" | grep -q "2" && ok "rc_profile=2 applied" || { bad "rc_profile readback: $V"; SECT_FAIL=$((SECT_FAIL+1)); }
else
	SECT_FAIL=$((SECT_FAIL+1))
fi

echo
echo "== 11) v3: mini-NetFlow (44B reply, was borderline) =="
readf netflow || SECT_FAIL=$((SECT_FAIL+1))
echo "    (populate with traffic; write 1 to reset)"

echo
echo "== 12) v3: channel NF survey (now 38B reply — former 64B-bare killer) =="
if OUT=$(timeout 5 cat "$D/chan_survey" 2>&1); then
	ok "chan_survey read OK"
	echo "$OUT" | sed 's/^/    | /'
	# sentinel honesty: nf values that are exactly 0 with count>0 are suspicious-fresh
	health "after-chan_survey" || { echo; echo "== SUITE ABORTED =="; exit 1; }
else
	bad "chan_survey read"; SECT_FAIL=$((SECT_FAIL+1))
	health "after-chan_survey-fail" || { echo; echo "== SUITE ABORTED =="; exit 1; }
fi

echo
echo "== 13) v3: firmware auto-spectral (on/off round trip) =="
if writef auto_spectral "on"; then
	V=$(timeout 5 cat "$D/auto_spectral" 2>&1) || V="READ-FAIL"
	echo "$V" | sed 's/^/    | /'
	writef auto_spectral "off" || SECT_FAIL=$((SECT_FAIL+1))
	readf auto_spectral >/dev/null || SECT_FAIL=$((SECT_FAIL+1))
else
	SECT_FAIL=$((SECT_FAIL+1))
fi

echo
echo "== 14) v3: black-box recorder =="
if writef blackbox 0; then
	readf blackbox || SECT_FAIL=$((SECT_FAIL+1))
	writef blackbox 1 || SECT_FAIL=$((SECT_FAIL+1))
	readf blackbox >/dev/null || SECT_FAIL=$((SECT_FAIL+1))
else
	SECT_FAIL=$((SECT_FAIL+1))
fi

echo
echo "== 15) firstep ANI-lite: write then READ BACK from the register =="
if OUT=$(timeout 5 cat "$D/firstep" 2>&1); then
	ok "firstep initial read"
	echo "$OUT" | sed 's/^/    | /'
	if echo "$OUT" | grep -q "find_sig_firstep=63"; then
		bad "firstep initial read shows find_sig_firstep=63 — SENTINEL of 0xFFFFFFFF, not a real register value"
		SECT_FAIL=$((SECT_FAIL+1))
	fi
else
	bad "firstep read"; SECT_FAIL=$((SECT_FAIL+1))
fi
if writef firstep 1; then
	V=$(timeout 5 cat "$D/firstep" 2>&1) || V="READ-FAIL"
	echo "$V" | sed 's/^/    | /'
	if echo "$V" | grep -q "find_sig_firstep=63\|level=0"; then
		bad "firstep=1 NOT reflected in register readback (sentinel or unchanged)"
		SECT_FAIL=$((SECT_FAIL+1))
	else
		ok "firstep=1 reflected in register readback"
	fi
	writef firstep 0 || SECT_FAIL=$((SECT_FAIL+1))
	readf firstep >/dev/null || SECT_FAIL=$((SECT_FAIL+1))
else
	SECT_FAIL=$((SECT_FAIL+1))
fi

echo
echo "== 16) v4: thermal governor (manual cap round trip) =="
if OUT=$(timeout 5 cat "$D/thermal" 2>&1); then
	ok "thermal read OK"
	echo "$OUT" | sed 's/^/    | /'
else
	bad "thermal read"; SECT_FAIL=$((SECT_FAIL+1))
fi
if writef thermal "cap 48"; then
	sleep 2
	V=$(timeout 5 cat "$D/thermal" 2>&1) || V="READ-FAIL"
	echo "$V" | sed 's/^/    | /'
	echo "$V" | grep -q "effective=48" && ok "effective dropped to 48 (PA heat reduction active)" \
		|| { bad "effective did NOT drop to 48"; SECT_FAIL=$((SECT_FAIL+1)); }
	writef thermal "cap 255" || SECT_FAIL=$((SECT_FAIL+1))
	V=$(timeout 5 cat "$D/thermal" 2>&1) || V="READ-FAIL"
	echo "$V" | grep -q "cap=255" && ok "manual cap removed" || { bad "cap readback: $V"; SECT_FAIL=$((SECT_FAIL+1)); }
else
	SECT_FAIL=$((SECT_FAIL+1))
fi

echo
echo "== 17) final WMI health + target diag =="
health "final" || true
readf custom_ping >/dev/null || true

# restore NM management
nmcli device set "$IFACE" managed yes 2>/dev/null && echo "-- NM managed: restored" || echo "-- WARN: NM restore failed"

echo
echo "=================================================================="
echo " VERIFICATION SUMMARY: PASS=$PASS FAIL=$FAIL"
if [ "$FAIL" -eq 0 ]; then
	echo " ALL CHECKS PASSED"
	echo "=================================================================="
	exit 0
else
	echo " FAILED STEPS:"
	for s in "${FAILED_STEPS[@]}"; do echo "   - $s"; done
	echo "=================================================================="
	exit 1
fi
