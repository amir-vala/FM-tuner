# Virtual Radio Tuner for Windows Media Center (WMC)

[فارسی در پایین]

## English
This project emulates a hardware network tuner (HDHomeRun protocol) to allow Windows Media Center on Windows 7 to play online radio streams. Since WMC typically requires a physical tuner for radio, this virtual approach bypasses those limitations by presenting online streams as audio-only TV channels.

### Features
- **HDHomeRun Emulation**: Seamless integration with WMC as a network tuner.
- **FFmpeg Powered**: High compatibility with various online stream formats (HLS, MP3, etc.).
- **Portable**: No installation required; runs from a single directory.
- **Easy Configuration**: Manage your stations using a simple `stations.json` file.

### How to Use
1. **Prepare FFmpeg**: Place `ffmpeg.exe` in the same directory as `VirtualTuner.exe`.
2. **Configure Stations**: Edit `stations.json` to add your favorite radio links.
   ```json
   {
       "guideName": "Radio Name",
       "guideNumber": "101",
       "url": "https://stream-url-here"
   }
   ```
3. **Run the App**: Start `VirtualTuner.exe`. Ensure you allow it through the Windows Firewall (UDP 65001 and HTTP 8080).
4. **WMC Setup**:
   - Open Windows Media Center.
   - Go to **Tasks > Settings > TV > Set Up TV Signal**.
   - WMC should detect a "Digital Antenna" or "HDHomeRun" tuner on the network.
   - Complete the setup and your stations will appear in the **TV Guide**.

---

## فارسی
این پروژه با شبیه‌سازی یک تیونر سخت‌افزاری تحت شبکه (پروتکل HDHomeRun)، به مدیا سنتر ویندوز ۷ (WMC) اجازه می‌دهد تا استریم‌های آنلاین رادیویی را پخش کند. از آنجایی که WMC به صورت پیش‌فرض برای بخش رادیو به سخت‌افزار واقعی نیاز دارد، این برنامه با نمایش استریم‌ها به عنوان «کانال‌های تلویزیونی فقط صوتی»، این محدودیت را دور می‌زند.

### ویژگی‌ها
- **شبیه‌سازی HDHomeRun**: اتصال بدون نقص به WMC به عنوان یک تیونر شبکه.
- **قدرت گرفته از FFmpeg**: پشتیبانی بالا از انواع فرمت‌های استریم آنلاین (HLS، MP3 و غیره).
- **پرتابل (قابل حمل)**: نیاز به نصب ندارد و از یک پوشه مستقل اجرا می‌شود.
- **تنظیمات آسان**: مدیریت ایستگاه‌ها از طریق فایل ساده `stations.json`.

### نحوه استفاده
۱. **آماده‌سازی FFmpeg**: فایل `ffmpeg.exe` را در کنار فایل اجرایی برنامه (`VirtualTuner.exe`) قرار دهید.
۲. **تنظیم ایستگاه‌ها**: فایل `stations.json` را ویرایش کرده و لینک‌های رادیویی خود را اضافه کنید.
   ```json
   {
       "guideName": "نام رادیو",
       "guideNumber": "101",
       "url": "لینک-استریم"
   }
   ```
۳. **اجرای برنامه**: فایل `VirtualTuner.exe` را اجرا کنید. مطمئن شوید که در فایروال ویندوز، اجازه دسترسی به برنامه داده شده است (پورت UDP 65001 و HTTP 8080).
۴. **تنظیمات در WMC**:
   - وارد Windows Media Center شوید.
   - به مسیر **Tasks > Settings > TV > Set Up TV Signal** بروید.
   - مدیا سنتر باید یک تیونر "Digital Antenna" یا "HDHomeRun" را در شبکه شناسایی کند.
   - مراحل نصب را ادامه دهید؛ ایستگاه‌های شما در بخش **TV Guide** ظاهر خواهند شد.
