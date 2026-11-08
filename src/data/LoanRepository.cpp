#include "data/LoanRepository.h"

#include <QSqlDatabase>
#include <QSqlQuery>
#include <QVariant>
#include <QVariantList>

#include "data/DbUtil.h"
#include "util/TextUtil.h"

QList<Loan> LoanRepository::list(LoanFilter filter, const QString& text, int limit) const {
    QString sql =
        "SELECT l.id, l.book_id, l.member_id, b.title, m.full_name, l.loan_date, l.due_date, l.return_date"
        " FROM loans l JOIN books b ON b.id = l.book_id JOIN members m ON m.id = l.member_id WHERE ";
    QVariantList binds;

    switch (filter) {
        case LoanFilter::Active:
            sql += "l.return_date IS NULL";
            break;
        case LoanFilter::Overdue:
            sql += "l.return_date IS NULL AND l.due_date < ?";
            binds << QDate::currentDate().toString(Qt::ISODate);
            break;
        case LoanFilter::Returned:
            sql += "l.return_date IS NOT NULL";
            break;
        case LoanFilter::All:
            sql += "1 = 1";
            break;
    }

    const QString t = TextUtil::normalize(text);
    if (!t.isEmpty()) {
        sql +=
            " AND (b.title LIKE ? ESCAPE '\\' OR m.full_name LIKE ? ESCAPE '\\'"
            " OR m.personnel_code LIKE ? ESCAPE '\\')";
        const QString like = TextUtil::likePattern(t);
        binds << like << like << TextUtil::likePattern(TextUtil::toLatinDigits(t));
    }
    sql += " ORDER BY l.id DESC";
    if (limit > 0) sql += QString(" LIMIT %1").arg(limit);

    QSqlQuery q;
    q.prepare(sql);
    for (const QVariant& v : binds) q.addBindValue(v);

    QList<Loan> out;
    if (!q.exec()) return out;
    while (q.next()) {
        Loan l;
        l.id = q.value(0).toLongLong();
        l.bookId = q.value(1).toLongLong();
        l.memberId = q.value(2).toLongLong();
        l.bookTitle = q.value(3).toString();
        l.memberName = q.value(4).toString();
        l.loanDate = QDate::fromString(q.value(5).toString(), Qt::ISODate);
        l.dueDate = QDate::fromString(q.value(6).toString(), Qt::ISODate);
        l.returnDate = QDate::fromString(q.value(7).toString(), Qt::ISODate);
        out.append(l);
    }
    return out;
}

bool LoanRepository::borrow(qint64 bookId, qint64 memberId, const QDate& dueDate,
                            QString* error) const {
    QSqlDatabase db = QSqlDatabase::database();
    db.transaction();

    QSqlQuery q;
    q.prepare(
        "SELECT b.copies - (SELECT COUNT(*) FROM loans l WHERE l.book_id = b.id AND l.return_date IS NULL)"
        " FROM books b WHERE b.id = ?");
    q.addBindValue(bookId);
    if (!q.exec() || !q.next()) {
        db.rollback();
        DbUtil::setError(error, "کتاب پیدا نشد.");
        return false;
    }
    if (q.value(0).toInt() <= 0) {
        db.rollback();
        DbUtil::setError(error, "همه‌ی نسخه‌های این کتاب در حال حاضر امانت داده شده است.");
        return false;
    }

    QSqlQuery ins;
    ins.prepare("INSERT INTO loans(book_id, member_id, loan_date, due_date) VALUES(?,?,?,?)");
    ins.addBindValue(bookId);
    ins.addBindValue(memberId);
    ins.addBindValue(QDate::currentDate().toString(Qt::ISODate));
    ins.addBindValue(dueDate.toString(Qt::ISODate));
    if (!ins.exec()) {
        db.rollback();
        DbUtil::setError(error, ins);
        return false;
    }
    db.commit();
    return true;
}

bool LoanRepository::giveBack(qint64 loanId, QString* error) const {
    QSqlQuery q;
    q.prepare("UPDATE loans SET return_date = ? WHERE id = ? AND return_date IS NULL");
    q.addBindValue(QDate::currentDate().toString(Qt::ISODate));
    q.addBindValue(loanId);
    if (!q.exec()) {
        DbUtil::setError(error, q);
        return false;
    }
    if (q.numRowsAffected() == 0) {
        DbUtil::setError(error, "این امانت قبلاً بازگردانده شده است.");
        return false;
    }
    return true;
}
