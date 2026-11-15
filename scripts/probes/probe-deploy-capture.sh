#!/usr/bin/env bash
# probe-deploy-capture.sh — capture USB EP traffic while deploy.sh runs,
# to see how far the target gets during boot (READY msg on EP3 or silence).
set -u
B="${BUILD_DIR:-$HOME/ath9k-fw-build}"
[ "$(id -u)" -eq 0 ] || { echo "run with sudo"; exit 1; }
MON=/tmp/mon-deploy.txt
modprobe usbmon 2>/dev/null || true
: > "$MON"
(timeout 60 cat /sys/kernel/debug/usb/usbmon/2u > "$MON" 2>/dev/null) &
MONPID=$!
sleep 0.5
"$B/deploy.sh" 2>&1 | tail -15
sleep 1
kill "$MONPID" 2>/dev/null; wait "$MONPID" 2>/dev/null
echo "=== usbmon: traffic per endpoint (dev on 2-1.2) ==="
DEVNUM=$(cat /sys/bus/usb/devices/2-1.2/devnum 2>/dev/null || echo "?")
python3 - "$DEVNUM" "$MON" <<'EOF'
import re,sys
dev,mon=sys.argv[1],sys.argv[2]
from collections import Counter
c=Counter(); data3=[]
for line in open(mon):
    m=re.match(r'^[0-9a-f]+ (\d+) ([SC]) ([ZXSCEBIC])([io]):(\d+):(\d+):(\d+) (.*)$', line.strip())
    if not m: continue
    ts,ev,t,di,bus,d,ep,rest=m.groups()
    if d!=dev: continue
    c[(ev,di,ep)]+=1
    data=rest.split("=",1)[1].strip().replace(" ","") if "=" in rest else ""
    if ep=="3" and ev=="C" and di=="i" and data:
        data3.append(data[:56])
print(f"dev={dev}:")
for k,v in sorted(c.items()): print(f"  {k}: {v}")
print("EP3 IN data samples (first 8):")
for d in data3[:8]: print(f"  {d}")
EOF
echo "=== dmesg tail ==="
dmesg | tail -12
