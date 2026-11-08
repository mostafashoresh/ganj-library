#pragma once
#include <QList>
#include <QString>

#include "core/Models.h"

struct ImportResult {
    QList<Book> books;
    int skippedNoTitle = 0;   // rows that had data but no book title
    bool hadHeader = false;
    QString error;            // non-empty on failure
};

// Reads a list of books from .xlsx (or .csv). Only the title column is required.
// If the first row contains recognizable headers (نام کتاب، نویسنده، ناشر، ...) they are used
// to map the columns; otherwise columns are taken in this order:
// title, author, publisher, isbn, year, category, location, copies
class ExcelImporter {
public:
    static ImportResult readBooks(const QString& path);
    static bool writeCsvTemplate(const QString& path, QString* error = nullptr);
};
