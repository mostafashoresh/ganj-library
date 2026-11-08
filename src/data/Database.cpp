#include "data/Database.h"

#include <QDir>
#include <QFile>
#include <QSqlDatabase>
#include <QSqlError>
#include <QSqlQuery>
#include <QStandardPaths>
#include <QStringList>

#include "data/DbUtil.h"

namespace {

// Schema version 1. To change the schema later, add a schemaV2() and a new block in migrate().
QStringList schemaV1() {
    return {
        "CREATE TABLE books ("
        " id INTEGER PRIMARY KEY AUTOINCREMENT,"
        " title TEXT NOT NULL,"
        " author TEXT NOT NULL DEFAULT '',"
        " publisher TEXT NOT NULL DEFAULT '',"
        " isbn TEXT NOT NULL DEFAULT '',"
        " year TEXT NOT NULL DEFAULT '',"
        " category TEXT NOT NULL DEFAULT '',"
        " location TEXT NOT NULL DEFAULT '',"
        " copies INTEGER NOT NULL DEFAULT 1 CHECK (copies >= 1),"
        " created_at TEXT NOT NULL DEFAULT CURRENT_TIMESTAMP)",
        "CREATE INDEX idx_books_title ON books(title)",
        "CREATE INDEX idx_books_author ON books(author)",

        "CREATE TABLE members ("
        " id INTEGER PRIMARY KEY AUTOINCREMENT,"
        " full_name TEXT NOT NULL,"
        " personnel_code TEXT NOT NULL DEFAULT '',"
        " unit TEXT NOT NULL DEFAULT '',"
        " phone TEXT NOT NULL DEFAULT '',"
        " created_at TEXT NOT NULL DEFAULT CURRENT_TIMESTAMP)",
        "CREATE INDEX idx_members_name ON members(full_name)",

        "CREATE TABLE loans ("
        " id INTEGER PRIMARY KEY AUTOINCREMENT,"
        " book_id INTEGER NOT NULL REFERENCES books(id) ON DELETE RESTRICT,"
        " member_id INTEGER NOT NULL REFERENCES members(id) ON DELETE RESTRICT,"
        " loan_date TEXT NOT NULL,"
        " due_date TEXT NOT NULL,"
        " return_date TEXT)",
        "CREATE INDEX idx_loans_book ON loans(book_id)",
        "CREATE INDEX idx_loans_member ON loans(member_id)",
        "CREATE INDEX idx_loans_open ON loans(return_date)",
    };
}

}  // namespace

QString Database::path() {
    const QString dir = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    return dir + "/ganj.db";
}

bool Database::open(QString* error) {
    const QString dir = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    if (!QDir().mkpath(dir)) {
        DbUtil::setError(error, "ساخت پوشه‌ی ذخیره‌سازی اطلاعات ممکن نشد: " + dir);
        return false;
    }

    QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE");
    db.setDatabaseName(path());
    if (!db.open()) {
        DbUtil::setError(error, db.lastError().text());
        return false;
    }

    QSqlQuery pragma(db);
    pragma.exec("PRAGMA foreign_keys = ON");
    pragma.exec("PRAGMA journal_mode = WAL");
    pragma.exec("PRAGMA synchronous = NORMAL");

    return migrate(error);
}

bool Database::migrate(QString* error) {
    QSqlDatabase db = QSqlDatabase::database();
    QSqlQuery q(db);

    if (!q.exec("PRAGMA user_version") || !q.next()) {
        DbUtil::setError(error, q);
        return false;
    }
    const int version = q.value(0).toInt();

    if (version < 1) {
        db.transaction();
        for (const QString& statement : schemaV1()) {
            if (!q.exec(statement)) {
                DbUtil::setError(error, q);
                db.rollback();
                return false;
            }
        }
        q.exec("PRAGMA user_version = 1");
        db.commit();
    }
    return true;
}

bool Database::backup(const QString& destination, QString* error) {
    QSqlQuery q(QSqlDatabase::database());
    q.exec("PRAGMA wal_checkpoint(TRUNCATE)");   // flush everything into the main file
    if (QFile::exists(destination)) QFile::remove(destination);
    if (!QFile::copy(path(), destination)) {
        DbUtil::setError(error, "کپی فایل پشتیبان انجام نشد.");
        return false;
    }
    return true;
}
