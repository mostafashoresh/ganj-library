#pragma once
#include <QList>
#include <QString>

#include "core/Models.h"

struct ImportSummary {
    int added = 0;
    int duplicates = 0;
    int failed = 0;
};

class BookRepository {
public:
    QList<Book> search(const QString& text = QString()) const;
    bool add(Book& book, QString* error = nullptr) const;
    bool update(const Book& book, QString* error = nullptr) const;
    bool remove(qint64 id, QString* error = nullptr) const;
    ImportSummary addMany(const QList<Book>& books) const;
};
