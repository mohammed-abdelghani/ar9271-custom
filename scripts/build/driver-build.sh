#!/usr/bin/env bash
# Build the patched ath9k_htc.ko + ath9k_common.ko from the sources shipped in
# this repo (sources/linux-6.12.116-ath9k — already modified, see patches/ for
# the delta vs upstream).
# The build runs in a space-free mirror (override with BUILD_DIR) because the
# repo may be cloned to a path containing spaces.
set -euo pipefail

HERE="$(cd "$(dirname "$0")" && pwd)"
SRC="$(cd "$HERE/../../sources/linux-6.12.116-ath9k/drivers/net/wireless/ath" && pwd)"
B="${BUILD_DIR:-$HOME/ath9k-fw-build}"    # space-free mirror + output dir
ATH9K=$B/driver/ath/ath9k

[ -d "$SRC" ] || { echo "FATAL: driver sources not found at $SRC" >&2; exit 1; }
[ -d "/lib/modules/$(uname -r)/build" ] || { echo "FATAL: kernel headers for $(uname -r) not installed (linux-headers-$(uname -r))" >&2; exit 1; }

# sync the (already-patched) sources into the mirror
rsync -a "$SRC/" "$B/driver/ath/"

cp "$HERE/ath9k-htc.kbuild" "$ATH9K/Kbuild"

make -C "/lib/modules/$(uname -r)/build" M="$ATH9K" modules

cp "$ATH9K/ath9k_htc.ko" "$B/ath9k_htc.ko"
cp "$ATH9K/ath9k_common.ko" "$B/ath9k_common.ko"
echo "=== DRIVER BUILD OK: $B/ath9k_htc.ko + $B/ath9k_common.ko ==="
