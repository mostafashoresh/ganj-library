#pragma once
#include <QString>

namespace TextUtil {

// Trims, collapses whitespace, converts Arabic ي/ك to Persian ی/ک
QString normalize(const QString& text);

QString toLatinDigits(QString text);     // ۱۲۳ / ١٢٣ -> 123
QString toPersianDigits(QString text);   // 123 -> ۱۲۳
QString fa(qint64 number);               // number -> Persian digits

// Builds a LIKE pattern "%text%" with %, _ and \ escaped (use with ESCAPE '\')
QString likePattern(const QString& text);

}  // namespace TextUtil
