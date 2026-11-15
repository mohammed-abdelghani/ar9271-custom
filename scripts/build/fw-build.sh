#!/usr/bin/env bash
# Build the Xtensa toolchain + patched ath9k_htc firmware from the sources
# shipped in this repo (sources/open-ath9k-htc-firmware — already modified,
# see patches/ for the delta vs upstream).
# Runs in a space-free mirror (override with BUILD_DIR) because the upstream
# Makefile breaks on paths with spaces (BASEDIR=$(shell pwd)).
set -euo pipefail

HERE="$(cd "$(dirname "$0")" && pwd)"
SRC="$(cd "$HERE/../../sources/open-ath9k-htc-firmware" && pwd)"
M="${BUILD_DIR:-$HOME/ath9k-fw-build}"    # space-free mirror + output dir
FW=$M/fw/open-ath9k-htc-firmware
DL=$FW/toolchain/dl

[ -d "$SRC" ] || { echo "FATAL: firmware sources not found at $SRC" >&2; exit 1; }

# --- 1) sync the (already-patched) sources into the mirror ----------------
rsync -a "$SRC/" "$FW/"

# --- 2) fetch toolchain tarballs from a stable mirror (kernel.org) --------
mkdir -p "$DL"
dl() {
	local url=$1 out=$2 sum=$3
	if [ -f "$out" ] && echo "$sum  $out" | sha256sum -c --status 2>/dev/null; then
		echo "[cached] $out"
		return
	fi
	rm -f "$out"
	for i in 1 2 3 4 5; do
		if curl -fL --retry 3 --retry-delay 3 -C - -o "$out" "$url"; then
			if echo "$sum  $out" | sha256sum -c --status; then
				echo "[ok] $out"
				return
			fi
		fi
		echo "attempt $i for $out failed, retrying..."
		sleep 5
	done
	echo "FATAL: could not fetch $url" >&2
	exit 1
}

# --- 3) build toolchain, then firmware ------------------------------------
cd "$FW"
make toolchain
make -C target_firmware

cp target_firmware/htc_9271.fw "$M/htc_9271.fw"
echo "=== FW BUILD OK: $M/htc_9271.fw ==="
