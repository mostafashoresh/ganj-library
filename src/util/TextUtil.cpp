#include "util/TextUtil.h"

namespace TextUtil {

QString normalize(const QString& text) {
    QString r = text;
    r.replace(QChar(0x064A), QChar(0x06CC));  // ي -> ی
    r.replace(QChar(0x0649), QChar(0x06CC));  // ى -> ی
    r.replace(QChar(0x0643), QChar(0x06A9));  // ك -> ک
    return r.simplified();
}

QString toLatinDigits(QString text) {
    for (QChar& c : text) {
        const ushort u = c.unicode();
        if (u >= 0x06F0 && u <= 0x06F9)
            c = QChar(int('0' + (u - 0x06F0)));
        else if (u >= 0x0660 && u <= 0x0669)
            c = QChar(int('0' + (u - 0x0660)));
    }
    return text;
}

QString toPersianDigits(QString text) {
    for (QChar& c : text) {
        const ushort u = c.unicode();
        if (u >= '0' && u <= '9')
            c = QChar(int(0x06F0 + (u - '0')));
    }
    return text;
}

QString fa(qint64 number) {
    return toPersianDigits(QString::number(number));
}

QString likePattern(const QString& text) {
    QString r = text;
    r.replace("\\", "\\\\");
    r.replace("%", "\\%");
    r.replace("_", "\\_");
    return "%" + r + "%";
}

}  // namespace TextUtil
