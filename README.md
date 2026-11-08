# سامانه مدیریت کتابخانه گنج
تیپ ۱۱۶ پیاده مکانیزه شهید سرلشگر صفوی

برنامه‌ی دسکتاپ ویندوز با C++‎ و Qt 6 — بانک اطلاعات SQLite، رابط راست‌به‌چین فارسی، تاریخ شمسی.

## امکانات
- لودر شروع برنامه با نام سامانه و نام تیپ
- داشبورد آمار (عناوین، نسخه‌ها، امانت فعال، تأخیر، اعضا) و پشتیبان‌گیری از بانک اطلاعات
- کتاب‌ها: افزودن دستی، ویرایش، حذف، جستجوی سریع، ورود گروهی از اکسل (xlsx) یا CSV
  - فقط «نام کتاب» الزامی است؛ بقیه‌ی فیلدها اختیاری‌اند
- اعضا: نام، کد پرسنلی، یگان، تلفن
- امانت و بازگشت: مدت امانت، سررسید شمسی، نمایش تأخیر، جلوگیری از امانت نسخه‌ی تمام‌شده

## فرمت فایل اکسل
ردیف اول عنوان ستون‌ها (ترتیب مهم نیست):
`نام کتاب | نویسنده | ناشر | شابک | سال انتشار | دسته‌بندی | قفسه | تعداد`
در برنامه، دکمه‌ی «؟» کنار «ورود از اکسل» یک فایل نمونه می‌دهد.

## ساخت فایل نصب (GanjLibrarySetup.exe)
### روش ۱ — خودکار با GitHub (بدون نصب هیچ‌چیز روی سیستم خودتان)
1. پوشه‌ی پروژه را در یک مخزن GitHub بگذارید و push کنید.
2. تب Actions ← «Build Windows installer» ← بعد از پایان، فایل `GanjLibrarySetup` را از Artifacts دانلود کنید.

### روش ۲ — روی ویندوز خودتان
پیش‌نیازها: Visual Studio 2022 (Desktop development with C++)، CMake، Qt 6.5 (MSVC 64-bit) و Inno Setup 6.
سپس `build_windows.bat` را اجرا کنید؛ خروجی در `dist\GanjLibrarySetup.exe` است.

فایل نصب خودش Qt و Microsoft Visual C++ Runtime را (در صورت نبودن) نصب می‌کند؛ کاربر نهایی به هیچ نصب جداگانه‌ای نیاز ندارد.

## ساختار پروژه
```
src/
  core/      Models.h              ساختارهای داده (Book, Member, Loan)
  util/      TextUtil, Jalali      نرمال‌سازی متن فارسی، تاریخ شمسی
  data/      Database + Repository ها (فقط اینجا SQL نوشته شده)
  services/  ExcelImporter, StatsService
  ui/        MainWindow, SplashWidget, Theme
  ui/pages/  Dashboard, Books, Members, Loans
  ui/dialogs/ BookDialog, MemberDialog, BorrowDialog
resources/   style.qss, آیکون، (fonts اختیاری)
installer/   ganj.iss (Inno Setup)
```

## محل ذخیره‌ی اطلاعات
`%APPDATA%\Ganj\Ganj\ganj.db` — با حذف برنامه پاک نمی‌شود. برای انتقال به سیستم دیگر از دکمه‌ی «پشتیبان‌گیری» استفاده کنید.

## فونت (اختیاری)
فایل‌های Vazirmatn را در `resources/fonts` بگذارید تا خودکار در نصب‌کننده قرار بگیرد.
