#pragma once
#include <QDate>
#include <QList>
#include <QString>

#include "core/Models.h"

enum class LoanFilter { Active, Overdue, Returned, All };

class LoanRepository {
public:
    QList<Loan> list(LoanFilter filter, const QString& text = QString(), int limit = 0) const;
    bool borrow(qint64 bookId, qint64 memberId, const QDate& dueDate, QString* error = nullptr) const;
    bool giveBack(qint64 loanId, QString* error = nullptr) const;
};
