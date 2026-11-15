#!/usr/bin/env bash
# apply-stability-tuning.sh — تحسين استقرار شريحة TL-WN422G v2 (AR9271) واستقرار وسرعة الإنترنت
# الاستخدام:  sudo bash apply-stability-tuning.sh
# كل التغييرات ملفات مستقلة قابلة للحذف — التفاصيل في docs/11-host-side-stability-tuning.md

set -uo pipefail

echo "== 1) NetworkManager: تعطيل توفير الطاقة للواي فاي عالمياً =="
cat > /etc/NetworkManager/conf.d/90-wifi-powersave-off.conf <<'EOF'
[connection]
# 2 = تعطيل توفير الطاقة (ping ثابت بلا نزلات، ومنع فقدان الاتصال عند الخمول)
wifi.powersave = 2
EOF
nmcli general reload 2>/dev/null || true

echo "== 2) modprobe.d: خيارات ath9k_htc الدائمة =="
cat > /etc/modprobe.d/ath9k_htc-stability.conf <<'EOF'
# use_dev_fw=1   : تحميل الفيرموير المطوّر (htc_9271-1.dev.0.fw) — يرجع لـ 1.4.0 تلقائياً إن غاب
#                  (بدون هذا السطر، use_dev_fw يفقد قيمته عند إعادة التشغيل لأن deploy.sh يضبطه مؤقتاً فقط)
# ps_enable=0    : منع توفير الطاقة داخل الفيرموير (CAM mode)
# btcoex_enable=0: تعطيل ازدواج البلوتوث (لا يوجد بلوتوث هنا)
# nohwcrypt=1    : التشفير على معالج الجهاز بدل الشريحة — يتجاوز مشاكل التشفير العتادي المعروفة في AR9271
options ath9k_htc use_dev_fw=1 ps_enable=0 btcoex_enable=0 nohwcrypt=1
EOF

echo "== 3) udev: تعطيل USB autosuspend للشريحة (يمنع فقدان الاتصال) =="
cat > /etc/udev/rules.d/91-ath9k-htc-stability.rules <<'EOF'
# AR9271 (TL-WN422G v2 / 0cf3:1006): منع النواة من تعليق الشريحة على منفذ USB
ACTION=="add", SUBSYSTEM=="usb", ATTR{idVendor}=="0cf3", ATTR{idProduct}=="1006", ATTR{power/autosuspend}="-1", ATTR{power/control}="on"
EOF
udevadm control --reload 2>/dev/null || true

echo "== 4) sysctl: TCP BBR + تضخيم البافرات =="
modprobe tcp_bbr 2>/dev/null && echo "tcp_bbr محمّل" || echo "tcp_bbr غير متاح — سيبقى cubic (مقبول)"
if sysctl -n net.ipv4.tcp_available_congestion_control | grep -qw bbr; then
  BBR_LINES="net.ipv4.tcp_congestion_control = bbr
net.core.default_qdisc = fq"
else
  BBR_LINES="# bbr غير متاح — cubic الافتراضي يبقى"
fi
cat > /etc/sysctl.d/91-wifi-internet-tuning.conf <<EOF
# BBR: يقاوم فقدان الحزم وbufferbloat على الوصلات اللاسلكية أفضل من cubic
$BBR_LINES
net.ipv4.tcp_slow_start_after_idle = 0
net.core.rmem_max = 16777216
net.core.wmem_max = 16777216
net.ipv4.tcp_rmem = 4096 131072 16777216
net.ipv4.tcp_wmem = 4096 16384 16777216
net.ipv4.tcp_mtu_probing = 1
EOF
sysctl -p /etc/sysctl.d/91-wifi-internet-tuning.conf

echo "== 5) تطبيق USB PM فوراً على الشريحة المركّبة حالياً =="
for d in /sys/bus/usb/devices/*; do
  if [ "$(cat "$d/idVendor" 2>/dev/null)" = "0cf3" ] && [ "$(cat "$d/idProduct" 2>/dev/null)" = "1006" ]; then
    echo on > "$d/power/control" && echo "USB PM: $d => on (autosuspend معطل)"
  fi
done

echo "== 6) تثبيت iw إن غاب (أداة الفحص القياسية للواي فاي) =="
command -v iw >/dev/null 2>&1 || apt-get install -y iw || echo "تعذر تثبيت iw — غير إلزامي"

echo
echo "== التحقق النهائي =="
sysctl net.ipv4.tcp_congestion_control net.core.default_qdisc net.ipv4.tcp_slow_start_after_idle 2>/dev/null
echo
echo "تم. خيارات الموديول في (2) تحتاج إعادة تحميل ath9k_htc لتطبق فعلياً:"
echo "  - إما إعادة تشغيل الجهاز"
echo "  - أو تشغيل: sudo deploy.sh من مجلد السكربتات (يعيد تحميل الموديول)"
