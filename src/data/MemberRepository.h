#pragma once
#include <QList>
#include <QString>

#include "core/Models.h"

class MemberRepository {
public:
    QList<Member> search(const QString& text = QString()) const;
    bool add(Member& member, QString* error = nullptr) const;
    bool update(const Member& member, QString* error = nullptr) const;
    bool remove(qint64 id, QString* error = nullptr) const;
};
