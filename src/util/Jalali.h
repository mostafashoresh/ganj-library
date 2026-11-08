#pragma once
#include <QDate>
#include <QString>

namespace Jalali {

QString format(const QDate& date);       // ۱۴۰۵/۰۸/۱۷  (empty string for invalid date)
QString longFormat(const QDate& date);   // یکشنبه ۱۷ آبان ۱۴۰۵

}  // namespace Jalali
