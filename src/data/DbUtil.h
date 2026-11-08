#pragma once
#include <QSqlError>
#include <QSqlQuery>
#include <QString>

namespace DbUtil {

inline void setError(QString* target, const QString& message) {
    if (target) *target = message;
}

inline void setError(QString* target, const QSqlQuery& query) {
    if (target) *target = query.lastError().text();
}

}  // namespace DbUtil
