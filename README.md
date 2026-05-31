# Virtual Radio Tuner for Windows Media Center (WMC)

[فارسی در پایین]

## English
This project emulates a hardware network tuner (HDHomeRun protocol) to allow Windows Media Center (WMC) on Windows 7 to play online radio streams. It converts online audio into a format WMC thinks is a TV channel.

### 🚀 Step-by-Step Guide (After Compilation)

1.  **Project Directory Structure**:
    After compiling, your folder should look like this:
    - `VirtualTuner.exe` (The compiled program)
    - `ffmpeg.exe` (Download from ffmpeg.org and place it here)
    - `stations.json` (The list of your radio stations)

2.  **Add Your Stations**:
    Open `stations.json` and add your links.
    - `guideNumber`: The channel number that will appear in WMC.
    - `guideName`: The name of the station.
    - `url`: The direct streaming link (HLS, MP3, etc.).

3.  **Run the Tuner**:
    Double-click `VirtualTuner.exe`. A console window will open. **Keep this window open** while using WMC.
    *Note: If Windows Firewall asks, allow "Private networks" access.*

4.  **Setup in Windows Media Center**:
    - Open Windows Media Center.
    - Go to **Tasks > Settings > TV > Set Up TV Signal**.
    - Confirm your region and let WMC search for tuners.
    - WMC should say "Found HDHomeRun" or "Digital Antenna". Select it.
    - If it asks for a zip code, enter any valid one.
    - Once the scan/setup is done, go to **TV > Guide**.
    - You will see your stations (e.g., channel 101, 102). Select one to start listening!

---

## فارسی: راهنمای گام‌به‌گام (بعد از کامپایل)

این پروژه یک تیونر مجازی می‌سازد تا مدیا سنتر ویندوز ۷ بتواند استریم‌های رادیویی آنلاین را پخش کند.

### 🚀 چطور از برنامه استفاده کنیم؟

۱. **ساختار پوشه برنامه**:
بعد از کامپایل، فایل‌های شما باید به این صورت در یک پوشه باشند:
- `VirtualTuner.exe` (فایل اصلی برنامه)
- `ffmpeg.exe` (باید آن را دانلود کرده و در کنار برنامه قرار دهید)
- `stations.json` (لیست شبکه‌های شما)

۲. **وارد کردن لیست رادیوها**:
فایل `stations.json` را باز کنید و لینک‌های خود را وارد کنید:
- `guideNumber`: شماره کانالی که در مدیا سنتر نمایش داده می‌شود.
- `guideName`: نام رادیو.
- `url`: لینک مستقیم استریم (مثلاً لینک‌های رادیو جوان یا رادیو ورزش).

۳. **اجرای برنامه**:
روی `VirtualTuner.exe` دو بار کلیک کنید تا اجرا شود. یک پنجره مشکی (کنسول) باز می‌شود. **تا زمانی که می‌خواهید رادیو گوش کنید، این پنجره را نبندید.**
*نکته: اگر فایروال ویندوز پیامی داد، تیک Allow را بزنید.*

۴. **تنظیمات در مدیا سنتر (WMC)**:
- برنامه Windows Media Center را باز کنید.
- به بخش **Tasks** و سپس **Settings** بروید. وارد **TV** شده و گزینه **Set Up TV Signal** را بزنید.
- مراحل را جلو بروید تا مدیا سنتر دنبال سخت‌افزار بگردد.
- برنامه باید به طور خودکار تیونر مجازی ما را با نام "HDHomeRun" یا "Digital Antenna" پیدا کند. آن را انتخاب کنید.
- اگر از شما کد پستی (Zip Code) خواست، یک عدد وارد کنید (مثلاً ۱۲۳۴۵).
- بعد از اتمام تنظیمات، به بخش **TV** و سپس **Guide** بروید.
- لیست رادیوهای خود را (مثلاً کانال ۱۰۱) مشاهده می‌کنید. با انتخاب هر کدام، رادیو شروع به پخش می‌کند.

---
### Technical Notes / نکات فنی
- **Port UDP 65001**: Used for discovery. / برای شناسایی تیونر در شبکه.
- **Port HTTP 8080**: Used for metadata and streaming. / برای انتقال اطلاعات شبکه و پخش استریم.
- **Transcoding**: FFmpeg converts audio to MPEG-TS on the fly. / تبدیل خودکار صدا به فرمت قابل فهم برای ویندوز توسط FFmpeg.
