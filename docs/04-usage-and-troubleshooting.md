# 🛠️ الاستخدام واستكشاف الأخطاء — TL-WN422G v2 على لينكس

## 1️⃣ أوامر الاستخدام اليومي

### التحقق من وجود الكارت ورفع الواجهة

```bash
ip link show wlxf8d111b37c15            # عرض حالة الواجهة
sudo ip link set wlxf8d111b37c15 up     # رفع الواجهة
```

### الاتصال بشبكة واي فاي (عبر NetworkManager)

```bash
nmcli device wifi list ifname wlxf8d111b37c15              # مسح الشبكات المتاحة
sudo nmcli device wifi connect "اسم الشبكة" \
     password "كلمة السر" ifname wlxf8d111b37c15           # الاتصال بشبكة
nmcli device disconnect wlxf8d111b37c15                    # فصل
```

### فحص جودة الإشارة

```bash
cat /proc/net/wireless          # مستوى الإشارة والضجيج لكل الواجهات
```

---

## 2️⃣ تشغيل Monitor Mode (للاختبارات الأمنية المصرّح بها فقط)

كروت الـ AR9271 من أنجح الكروت في مجال الاختبار الأمني — ادعم الـ Monitor Mode و Packet Injection بشكل مستقر عبر `ath9k_htc`.

> ⚠️ استخدم هذه القدرات فقط على شبكاتك أو بيئات معملية/اختبارات مُصرَّح بها (CTF، pentesting بعقد عمل… إلخ).

```bash
# 1) تأكد من تثبيت الأدوات (غير مثبّتة على هذا الجهاز حالياً)
sudo apt install iw aircrack-ng

# 2) إيقاف الخدمات التي تعيق الوضع الجديد (اختياري لكن مفضّل)
sudo systemctl stop NetworkManager wpa_supplicant

# 3) تحويل الكارت إلى Monitor Mode
sudo ip link set wlxf8d111b37c15 down
sudo iw dev wlxf8d111b37c15 set type monitor
sudo ip link set wlxf8d111b37c15 up

# 4) التحقق
iw dev                      # يجب أن تظهر Mode: Monitor
sudo airodump-ng wlxf8d111b37c15

# 5) الرجوع للوضع الطبيعي (Managed)
sudo ip link set wlxf8d111b37c15 down
sudo iw dev wlxf8d111b37c15 set type managed
sudo ip link set wlxf8d111b37c15 up
sudo systemctl start NetworkManager
```

---

## 3️⃣ استكشاف الأخطاء وإصلاحها

### ❌ المشكلة: الكارت لا يظهر نهائياً (لا يوجد `wlx…` في `ip link`)

1. تأكد من الجهاز على الـ USB:
   ```bash
   lsusb | grep -i 0cf3
   dmesg | grep -i ath9k
   ```
2. إن ظهرت رسالة مثل `failed to load firmware` أو `firmware htc_9271... failed`:
   ```bash
   sudo apt install firmware-atheros      # تثبيت الفيرموير
   sudo modprobe -r ath9k_htc && sudo modprobe ath9k_htc
   ```

### ❌ المشكلة: الكارت يُفصل أو يتجمد أثناء العمل

- جرّب منفذ **USB 2.0** بدل 3.0 (تداخل كهرومغناطيسي من منافذ 3.0 على تردد 2.4 GHz).
- تأكد أن `ps_enable=0` (إيقاف Power Save):
  ```bash
  cat /sys/module/ath9k_htc/parameters/ps_enable
  ```
- لو الاتصال ببروتوكولات معينة بيفشل، جرّب تعطيل تشفير العتاد:
  ```bash
  echo "options ath9k_htc nohwcrypt=1" | sudo tee /etc/modprobe.d/ath9k_htc.conf
  ```

### ❌ المشكلة: الإشارة ضعيفة رغم الهوائي 4 dBi

- تأكد أن الهوائي الخارجي مربوط بإحكام (موصل RP-SMA — يُفصل ويركّب بسهولة).
- الجهاز يعمل على 2.4 GHz فقط؛ القنوات 1 / 6 / 11 هي الأفضل (غير متداخلة).
- يُفضّل فصل الهوائي وإعادة تركبه للتأكد من عدم تلف الموصّل.

### ❌ المشكلة: لا يوجد شبكات تظهر في المسح

```bash
sudo ip link set wlxf8d111b37c15 down
sudo ip link set wlxf8d111b37c15 up
nmcli device wifi rescan ifname wlxf8d111b37c15
```
وإن استمرت المشكلة: أعد تحميل الدرايفَر:
```bash
sudo modprobe -r ath9k_htc && sleep 2 && sudo modprobe ath9k_htc
```

---

## 4️⃣ أوامر مرجعية سريعة

| الهدف | الأمر |
|---|---|
| وجود الجهاز | `lsusb \| grep 0cf3:1006` |
| الإنترفيس والدرايفر | `readlink /sys/class/net/wlxf8d111b37c15/device/driver` |
| إعادة تحميل الدرايفَر | `sudo modprobe -r ath9k_htc && sudo modprobe ath9k_htc` |
| الباراميترات | `ls /sys/module/ath9k_htc/parameters/` |
| حالة الاتصال | `nmcli device status` |

📅 آخر تحديث: 2026-11-13
