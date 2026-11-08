#pragma once
#include <QString>

// Opens the SQLite file in the user's AppData folder and keeps the schema up to date.
class Database {
public:
    static bool open(QString* error = nullptr);
    static QString path();
    static bool backup(const QString& destination, QString* error = nullptr);

private:
    static bool migrate(QString* error);
};
