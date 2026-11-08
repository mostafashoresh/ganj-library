#include "services/StatsService.h"

#include <QDate>
#include <QSqlQuery>
#include <QVariant>

namespace {

int scalar(const QString& sql, const QString& bind = QString()) {
    QSqlQuery q;
    q.prepare(sql);
    if (!bind.isEmpty()) q.addBindValue(bind);
    return (q.exec() && q.next()) ? q.value(0).toInt() : 0;
}

}  // namespace

Stats StatsService::compute() const {
    Stats s;
    s.titles = scalar("SELECT COUNT(*) FROM books");
    s.copies = scalar("SELECT COALESCE(SUM(copies), 0) FROM books");
    s.activeLoans = scalar("SELECT COUNT(*) FROM loans WHERE return_date IS NULL");
    s.members = scalar("SELECT COUNT(*) FROM members");
    s.overdue = scalar("SELECT COUNT(*) FROM loans WHERE return_date IS NULL AND due_date < ?",
                       QDate::currentDate().toString(Qt::ISODate));
    return s;
}
