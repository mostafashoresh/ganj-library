#include "util/Jalali.h"
#include "util/TextUtil.h"

namespace {

void toJalali(const QDate& date, int& jy, int& jm, int& jd) {
    static const int cum[] = {0, 31, 59, 90, 120, 151, 181, 212, 243, 273, 304, 334};
    const int gy = date.year();
    const int gm = date.month();
    const int gd = date.day();
    const int gy2 = (gm > 2) ? (gy + 1) : gy;
    long days = 355666 + (365L * gy) + ((gy2 + 3) / 4) - ((gy2 + 99) / 100) +
                ((gy2 + 399) / 400) + gd + cum[gm - 1];
    jy = -1595 + static_cast<int>(33 * (days / 12053));
    days %= 12053;
    jy += static_cast<int>(4 * (days / 1461));
    days %= 1461;
    if (days > 365) {
        jy += static_cast<int>((days - 1) / 365);
        days = (days - 1) % 365;
    }
    if (days < 186) {
        jm = 1 + static_cast<int>(days / 31);
        jd = 1 + static_cast<int>(days % 31);
    } else {
        jm = 7 + static_cast<int>((days - 186) / 30);
        jd = 1 + static_cast<int>((days - 186) % 30);
    }
}

}  // namespace

namespace Jalali {

QString format(const QDate& date) {
    if (!date.isValid()) return QString();
    int y, m, d;
    toJalali(date, y, m, d);
    const QString s = QString("%1/%2/%3")
                          .arg(y)
                          .arg(m, 2, 10, QChar('0'))
                          .arg(d, 2, 10, QChar('0'));
    return TextUtil::toPersianDigits(s);
}

QString longFormat(const QDate& date) {
    if (!date.isValid()) return QString();
    static const char* months[] = {"فروردین", "اردیبهشت", "خرداد", "تیر", "مرداد", "شهریور",
                                   "مهر", "آبان", "آذر", "دی", "بهمن", "اسفند"};
    // Qt: Monday = 1 ... Sunday = 7
    static const char* weekdays[] = {"دوشنبه", "سه\u200Cشنبه", "چهارشنبه", "پنجشنبه",
                                     "جمعه", "شنبه", "یکشنبه"};
    int y, m, d;
    toJalali(date, y, m, d);
    return QString("%1 %2 %3 %4")
        .arg(QString::fromUtf8(weekdays[date.dayOfWeek() - 1]))
        .arg(TextUtil::fa(d))
        .arg(QString::fromUtf8(months[m - 1]))
        .arg(TextUtil::fa(y));
}

}  // namespace Jalali
