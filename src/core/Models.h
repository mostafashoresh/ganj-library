#pragma once
#include <QDate>
#include <QString>

// ---- Plain data structures shared by every layer (no Qt Sql / Widgets here) ----

struct Book {
    qint64 id = 0;
    QString title;       // required
    QString author;      // everything below is optional
    QString publisher;
    QString isbn;
    QString year;
    QString category;
    QString location;    // shelf / place
    int copies = 1;
    int available = 1;   // computed: copies - active loans
};

struct Member {
    qint64 id = 0;
    QString fullName;    // required
    QString personnelCode;
    QString unit;
    QString phone;
    int activeLoans = 0; // computed
};

struct Loan {
    qint64 id = 0;
    qint64 bookId = 0;
    qint64 memberId = 0;
    QString bookTitle;
    QString memberName;
    QDate loanDate;
    QDate dueDate;
    QDate returnDate;

    bool isReturned() const { return returnDate.isValid(); }
    bool isOverdue() const {
        return !isReturned() && dueDate.isValid() && dueDate < QDate::currentDate();
    }
};

struct Stats {
    int titles = 0;
    int copies = 0;
    int activeLoans = 0;
    int members = 0;
    int overdue = 0;
};
