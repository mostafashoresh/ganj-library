#include "data/BookRepository.h"

#include <QSqlDatabase>
#include <QSqlQuery>
#include <QVariant>

#include "data/DbUtil.h"
#include "util/TextUtil.h"

namespace {

Book cleaned(Book b) {
    b.title = TextUtil::normalize(b.title);
    b.author = TextUtil::normalize(b.author);
    b.publisher = TextUtil::normalize(b.publisher);
    b.category = TextUtil::normalize(b.category);
    b.location = TextUtil::normalize(b.location);
    b.isbn = TextUtil::toLatinDigits(TextUtil::normalize(b.isbn));
    b.year = TextUtil::toLatinDigits(TextUtil::normalize(b.year));
    if (b.copies < 1) b.copies = 1;
    return b;
}

}  // namespace

QList<Book> BookRepository::search(const QString& text) const {
    QString sql =
        "SELECT b.id, b.title, b.author, b.publisher, b.isbn, b.year, b.category, b.location, b.copies,"
        " b.copies - (SELECT COUNT(*) FROM loans l WHERE l.book_id = b.id AND l.return_date IS NULL)"
        " FROM books b";

    const QString t = TextUtil::normalize(text);
    if (!t.isEmpty()) {
        sql +=
            " WHERE b.title LIKE ? ESCAPE '\\' OR b.author LIKE ? ESCAPE '\\'"
            " OR b.publisher LIKE ? ESCAPE '\\' OR b.category LIKE ? ESCAPE '\\'"
            " OR b.location LIKE ? ESCAPE '\\' OR b.isbn LIKE ? ESCAPE '\\'";
    }
    sql += " ORDER BY b.title COLLATE NOCASE";

    QSqlQuery q;
    q.prepare(sql);
    if (!t.isEmpty()) {
        const QString like = TextUtil::likePattern(t);
        for (int i = 0; i < 5; ++i) q.addBindValue(like);
        q.addBindValue(TextUtil::likePattern(TextUtil::toLatinDigits(t)));
    }

    QList<Book> out;
    if (!q.exec()) return out;
    while (q.next()) {
        Book b;
        b.id = q.value(0).toLongLong();
        b.title = q.value(1).toString();
        b.author = q.value(2).toString();
        b.publisher = q.value(3).toString();
        b.isbn = q.value(4).toString();
        b.year = q.value(5).toString();
        b.category = q.value(6).toString();
        b.location = q.value(7).toString();
        b.copies = q.value(8).toInt();
        b.available = q.value(9).toInt();
        out.append(b);
    }
    return out;
}

bool BookRepository::add(Book& book, QString* error) const {
    book = cleaned(book);
    if (book.title.isEmpty()) {
        DbUtil::setError(error, "نام کتاب الزامی است.");
        return false;
    }
    QSqlQuery q;
    q.prepare(
        "INSERT INTO books(title, author, publisher, isbn, year, category, location, copies)"
        " VALUES(?,?,?,?,?,?,?,?)");
    q.addBindValue(book.title);
    q.addBindValue(book.author);
    q.addBindValue(book.publisher);
    q.addBindValue(book.isbn);
    q.addBindValue(book.year);
    q.addBindValue(book.category);
    q.addBindValue(book.location);
    q.addBindValue(book.copies);
    if (!q.exec()) {
        DbUtil::setError(error, q);
        return false;
    }
    book.id = q.lastInsertId().toLongLong();
    book.available = book.copies;
    return true;
}

bool BookRepository::update(const Book& input, QString* error) const {
    const Book book = cleaned(input);
    if (book.title.isEmpty()) {
        DbUtil::setError(error, "نام کتاب الزامی است.");
        return false;
    }

    QSqlQuery check;
    check.prepare("SELECT COUNT(*) FROM loans WHERE book_id = ? AND return_date IS NULL");
    check.addBindValue(book.id);
    if (check.exec() && check.next() && check.value(0).toInt() > book.copies) {
        DbUtil::setError(error, "تعداد نسخه‌ها نمی‌تواند از تعداد امانت‌های فعال این کتاب کمتر باشد.");
        return false;
    }

    QSqlQuery q;
    q.prepare(
        "UPDATE books SET title=?, author=?, publisher=?, isbn=?, year=?, category=?, location=?, copies=?"
        " WHERE id=?");
    q.addBindValue(book.title);
    q.addBindValue(book.author);
    q.addBindValue(book.publisher);
    q.addBindValue(book.isbn);
    q.addBindValue(book.year);
    q.addBindValue(book.category);
    q.addBindValue(book.location);
    q.addBindValue(book.copies);
    q.addBindValue(book.id);
    if (!q.exec()) {
        DbUtil::setError(error, q);
        return false;
    }
    return true;
}

bool BookRepository::remove(qint64 id, QString* error) const {
    QSqlQuery check;
    check.prepare("SELECT COUNT(*) FROM loans WHERE book_id = ? AND return_date IS NULL");
    check.addBindValue(id);
    if (check.exec() && check.next() && check.value(0).toInt() > 0) {
        DbUtil::setError(error, "این کتاب در حال حاضر امانت داده شده و قابل حذف نیست.");
        return false;
    }

    QSqlDatabase db = QSqlDatabase::database();
    db.transaction();
    QSqlQuery history;
    history.prepare("DELETE FROM loans WHERE book_id = ?");   // only returned loans remain
    history.addBindValue(id);
    QSqlQuery q;
    q.prepare("DELETE FROM books WHERE id = ?");
    q.addBindValue(id);
    if (!history.exec() || !q.exec()) {
        DbUtil::setError(error, q.lastError().isValid() ? q.lastError().text()
                                                         : history.lastError().text());
        db.rollback();
        return false;
    }
    db.commit();
    return true;
}

ImportSummary BookRepository::addMany(const QList<Book>& books) const {
    ImportSummary summary;
    QSqlDatabase db = QSqlDatabase::database();
    db.transaction();

    QSqlQuery dup;
    dup.prepare("SELECT 1 FROM books WHERE title = ? AND author = ? LIMIT 1");

    for (Book b : books) {
        b = cleaned(b);
        if (b.title.isEmpty()) {
            ++summary.failed;
            continue;
        }
        dup.bindValue(0, b.title);
        dup.bindValue(1, b.author);
        const bool isDuplicate = dup.exec() && dup.next();
        dup.finish();
        if (isDuplicate) {
            ++summary.duplicates;
            continue;
        }
        if (add(b, nullptr))
            ++summary.added;
        else
            ++summary.failed;
    }
    db.commit();
    return summary;
}
