#include "services/ExcelImporter.h"

#include <QFile>
#include <QHash>
#include <QStringDecoder>
#include <QVector>
#include <QXmlStreamReader>
#include <private/qzipreader_p.h>

#include <algorithm>

#include "util/TextUtil.h"

namespace {

enum Field {
    FTitle, FAuthor, FPublisher, FIsbn, FYear, FCategory, FLocation, FCopies, FieldCount
};

int fieldForHeader(const QString& header) {
    QString k = TextUtil::normalize(header).toLower();
    k.remove(' ');
    k.remove(QChar(0x200C));
    k.remove(QChar(0x200F));
    k.remove(':');
    k.remove('*');

    static const QHash<QString, int> map = {
        {"نامکتاب", FTitle}, {"عنوانکتاب", FTitle}, {"عنوان", FTitle}, {"کتاب", FTitle},
        {"نام", FTitle}, {"title", FTitle}, {"bookname", FTitle}, {"name", FTitle},
        {"نویسنده", FAuthor}, {"مولف", FAuthor}, {"مؤلف", FAuthor}, {"نامنویسنده", FAuthor},
        {"author", FAuthor}, {"writer", FAuthor},
        {"ناشر", FPublisher}, {"انتشارات", FPublisher}, {"نامناشر", FPublisher},
        {"publisher", FPublisher},
        {"شابک", FIsbn}, {"isbn", FIsbn},
        {"سال", FYear}, {"سالانتشار", FYear}, {"سالچاپ", FYear}, {"year", FYear},
        {"دسته", FCategory}, {"دستهبندی", FCategory}, {"موضوع", FCategory},
        {"گروه", FCategory}, {"category", FCategory}, {"subject", FCategory},
        {"قفسه", FLocation}, {"محل", FLocation}, {"مکان", FLocation},
        {"محلقفسه", FLocation}, {"شمارهقفسه", FLocation}, {"location", FLocation},
        {"shelf", FLocation},
        {"تعداد", FCopies}, {"تعدادنسخه", FCopies}, {"نسخه", FCopies},
        {"count", FCopies}, {"copies", FCopies},
    };
    return map.value(k, -1);
}

// Excel often stores numbers like 9.78E+12 or 1399.0
QString cleanNumber(QString v) {
    v = TextUtil::toLatinDigits(v);
    bool ok = false;
    if (v.contains('E', Qt::CaseInsensitive) || v.endsWith(".0")) {
        const double d = v.toDouble(&ok);
        if (ok) return QString::number(d, 'f', 0);
    }
    return v;
}

int columnIndex(const QString& ref) {
    int col = 0;
    for (const QChar& c : ref) {
        if (!c.isLetter()) break;
        col = col * 26 + (c.toUpper().unicode() - 'A' + 1);
    }
    return col - 1;
}

QStringList readSharedStrings(QZipReader& zip) {
    QStringList out;
    const QByteArray data = zip.fileData("xl/sharedStrings.xml");
    if (data.isEmpty()) return out;

    QXmlStreamReader xml(data);
    QString current;
    bool inSi = false;
    while (!xml.atEnd()) {
        xml.readNext();
        if (xml.isStartElement()) {
            if (xml.name() == QLatin1String("si")) {
                inSi = true;
                current.clear();
            } else if (inSi && xml.name() == QLatin1String("t")) {
                current += xml.readElementText();
            }
        } else if (xml.isEndElement() && xml.name() == QLatin1String("si")) {
            out << current;
            inSi = false;
        }
    }
    return out;
}

QList<QStringList> readXlsx(const QString& path, QString* error) {
    QList<QStringList> rows;
    QZipReader zip(path);
    if (!zip.isReadable()) {
        *error = "فایل اکسل قابل خواندن نیست. فقط فرمت xlsx پشتیبانی می‌شود (نه xls قدیمی).";
        return rows;
    }

    QByteArray sheet = zip.fileData("xl/worksheets/sheet1.xml");
    if (sheet.isEmpty()) {
        QStringList names;
        for (const auto& info : zip.fileInfoList()) {
            if (info.filePath.startsWith("xl/worksheets/sheet") && info.filePath.endsWith(".xml"))
                names << info.filePath;
        }
        std::sort(names.begin(), names.end());
        if (!names.isEmpty()) sheet = zip.fileData(names.first());
    }
    if (sheet.isEmpty()) {
        *error = "هیچ برگه‌ای در فایل اکسل پیدا نشد.";
        return rows;
    }

    const QStringList shared = readSharedStrings(zip);

    QXmlStreamReader xml(sheet);
    QStringList current;
    while (!xml.atEnd()) {
        xml.readNext();
        if (xml.isStartElement()) {
            if (xml.name() == QLatin1String("row")) {
                current.clear();
            } else if (xml.name() == QLatin1String("c")) {
                const QString ref = xml.attributes().value("r").toString();
                const QString type = xml.attributes().value("t").toString();
                QString value;
                while (!xml.atEnd()) {
                    xml.readNext();
                    if (xml.isEndElement() && xml.name() == QLatin1String("c")) break;
                    if (xml.isStartElement()) {
                        if (xml.name() == QLatin1String("v"))
                            value = xml.readElementText();
                        else if (xml.name() == QLatin1String("t"))   // inline string
                            value += xml.readElementText();
                    }
                }
                if (type == "s") value = shared.value(value.toInt());
                const int col = columnIndex(ref);
                if (col >= 0) {
                    while (current.size() <= col) current << QString();
                    current[col] = value;
                }
            }
        } else if (xml.isEndElement() && xml.name() == QLatin1String("row")) {
            rows << current;
        }
    }
    return rows;
}

QList<QStringList> parseCsv(const QString& text) {
    QChar delim = ',';
    const QString firstLine = text.left(text.indexOf('\n'));
    if (firstLine.count(';') > firstLine.count(',')) delim = ';';
    if (firstLine.count('\t') > firstLine.count(delim)) delim = '\t';

    QList<QStringList> rows;
    QStringList row;
    QString field;
    bool inQuotes = false;
    for (int i = 0; i < text.size(); ++i) {
        const QChar c = text[i];
        if (inQuotes) {
            if (c == '"') {
                if (i + 1 < text.size() && text[i + 1] == '"') {
                    field += '"';
                    ++i;
                } else {
                    inQuotes = false;
                }
            } else {
                field += c;
            }
        } else if (c == '"') {
            inQuotes = true;
        } else if (c == delim) {
            row << field;
            field.clear();
        } else if (c == '\n' || c == '\r') {
            if (c == '\r' && i + 1 < text.size() && text[i + 1] == '\n') ++i;
            row << field;
            field.clear();
            rows << row;
            row.clear();
        } else {
            field += c;
        }
    }
    if (!field.isEmpty() || !row.isEmpty()) {
        row << field;
        rows << row;
    }
    return rows;
}

QList<QStringList> readCsv(const QString& path, QString* error) {
    QFile f(path);
    if (!f.open(QIODevice::ReadOnly)) {
        *error = "فایل باز نشد.";
        return {};
    }
    const QByteArray data = f.readAll();
    QString text = QString::fromUtf8(data);
    if (text.contains(QChar(0xFFFD))) {              // probably an old Windows-1256 CSV
        QStringDecoder decoder("Windows-1256");
        if (decoder.isValid()) text = decoder(data);
    }
    if (text.startsWith(QChar(0xFEFF))) text.remove(0, 1);   // BOM
    return parseCsv(text);
}

}  // namespace

ImportResult ExcelImporter::readBooks(const QString& path) {
    ImportResult res;
    const bool isCsv = path.endsWith(".csv", Qt::CaseInsensitive);
    const QList<QStringList> rows = isCsv ? readCsv(path, &res.error) : readXlsx(path, &res.error);
    if (!res.error.isEmpty()) return res;
    if (rows.isEmpty()) {
        res.error = "فایل خالی است.";
        return res;
    }

    QVector<int> headerMap;
    for (const QString& h : rows.first()) headerMap << fieldForHeader(h);
    res.hadHeader = headerMap.contains(FTitle);

    QVector<int> map;
    int start = 0;
    if (res.hadHeader) {
        map = headerMap;
        start = 1;
    } else {
        for (int i = 0; i < FieldCount; ++i) map << i;
    }

    for (int r = start; r < rows.size(); ++r) {
        const QStringList& row = rows[r];
        Book b;
        bool anyData = false;
        for (int c = 0; c < row.size() && c < map.size(); ++c) {
            const QString v = TextUtil::normalize(row[c]);
            if (v.isEmpty()) continue;
            anyData = true;
            switch (map[c]) {
                case FTitle: b.title = v; break;
                case FAuthor: b.author = v; break;
                case FPublisher: b.publisher = v; break;
                case FIsbn: b.isbn = cleanNumber(v); break;
                case FYear: b.year = cleanNumber(v); break;
                case FCategory: b.category = v; break;
                case FLocation: b.location = v; break;
                case FCopies: {
                    const int n = static_cast<int>(cleanNumber(v).toDouble());
                    b.copies = n > 0 ? n : 1;
                    break;
                }
                default: break;
            }
        }
        if (!anyData) continue;                       // empty row
        if (b.title.isEmpty()) {
            ++res.skippedNoTitle;
            continue;
        }
        res.books << b;
    }
    return res;
}

bool ExcelImporter::writeCsvTemplate(const QString& path, QString* error) {
    QFile f(path);
    if (!f.open(QIODevice::WriteOnly)) {
        if (error) *error = "ساخت فایل ممکن نشد.";
        return false;
    }
    const QString text =
        "نام کتاب,نویسنده,ناشر,شابک,سال انتشار,دسته‌بندی,قفسه,تعداد\r\n"
        "نمونه کتاب اول,نویسنده نمونه,انتشارات نمونه,9780000000001,1400,تاریخ,A-1,2\r\n"
        "فقط نام کتاب کافی است,,,,,,,\r\n";
    f.write("\xEF\xBB\xBF");                 // UTF-8 BOM so Excel shows Persian correctly
    f.write(text.toUtf8());
    return true;
}
