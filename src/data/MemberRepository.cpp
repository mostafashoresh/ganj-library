#include "data/MemberRepository.h"

#include <QSqlDatabase>
#include <QSqlQuery>
#include <QVariant>

#include "data/DbUtil.h"
#include "util/TextUtil.h"

namespace {

Member cleaned(Member m) {
    m.fullName = TextUtil::normalize(m.fullName);
    m.unit = TextUtil::normalize(m.unit);
    m.personnelCode = TextUtil::toLatinDigits(TextUtil::normalize(m.personnelCode));
    m.phone = TextUtil::toLatinDigits(TextUtil::normalize(m.phone));
    return m;
}

}  // namespace

QList<Member> MemberRepository::search(const QString& text) const {
    QString sql =
        "SELECT m.id, m.full_name, m.personnel_code, m.unit, m.phone,"
        " (SELECT COUNT(*) FROM loans l WHERE l.member_id = m.id AND l.return_date IS NULL)"
        " FROM members m";
    const QString t = TextUtil::normalize(text);
    if (!t.isEmpty()) {
        sql +=
            " WHERE m.full_name LIKE ? ESCAPE '\\' OR m.unit LIKE ? ESCAPE '\\'"
            " OR m.personnel_code LIKE ? ESCAPE '\\' OR m.phone LIKE ? ESCAPE '\\'";
    }
    sql += " ORDER BY m.full_name COLLATE NOCASE";

    QSqlQuery q;
    q.prepare(sql);
    if (!t.isEmpty()) {
        const QString like = TextUtil::likePattern(t);
        const QString digits = TextUtil::likePattern(TextUtil::toLatinDigits(t));
        q.addBindValue(like);
        q.addBindValue(like);
        q.addBindValue(digits);
        q.addBindValue(digits);
    }

    QList<Member> out;
    if (!q.exec()) return out;
    while (q.next()) {
        Member m;
        m.id = q.value(0).toLongLong();
        m.fullName = q.value(1).toString();
        m.personnelCode = q.value(2).toString();
        m.unit = q.value(3).toString();
        m.phone = q.value(4).toString();
        m.activeLoans = q.value(5).toInt();
        out.append(m);
    }
    return out;
}

bool MemberRepository::add(Member& member, QString* error) const {
    member = cleaned(member);
    if (member.fullName.isEmpty()) {
        DbUtil::setError(error, "نام عضو الزامی است.");
        return false;
    }
    QSqlQuery q;
    q.prepare("INSERT INTO members(full_name, personnel_code, unit, phone) VALUES(?,?,?,?)");
    q.addBindValue(member.fullName);
    q.addBindValue(member.personnelCode);
    q.addBindValue(member.unit);
    q.addBindValue(member.phone);
    if (!q.exec()) {
        DbUtil::setError(error, q);
        return false;
    }
    member.id = q.lastInsertId().toLongLong();
    return true;
}

bool MemberRepository::update(const Member& input, QString* error) const {
    const Member m = cleaned(input);
    if (m.fullName.isEmpty()) {
        DbUtil::setError(error, "نام عضو الزامی است.");
        return false;
    }
    QSqlQuery q;
    q.prepare("UPDATE members SET full_name=?, personnel_code=?, unit=?, phone=? WHERE id=?");
    q.addBindValue(m.fullName);
    q.addBindValue(m.personnelCode);
    q.addBindValue(m.unit);
    q.addBindValue(m.phone);
    q.addBindValue(m.id);
    if (!q.exec()) {
        DbUtil::setError(error, q);
        return false;
    }
    return true;
}

bool MemberRepository::remove(qint64 id, QString* error) const {
    QSqlQuery check;
    check.prepare("SELECT COUNT(*) FROM loans WHERE member_id = ? AND return_date IS NULL");
    check.addBindValue(id);
    if (check.exec() && check.next() && check.value(0).toInt() > 0) {
        DbUtil::setError(error, "این عضو کتاب بازنگردانده دارد و قابل حذف نیست.");
        return false;
    }

    QSqlDatabase db = QSqlDatabase::database();
    db.transaction();
    QSqlQuery history;
    history.prepare("DELETE FROM loans WHERE member_id = ?");
    history.addBindValue(id);
    QSqlQuery q;
    q.prepare("DELETE FROM members WHERE id = ?");
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
