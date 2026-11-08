#include "ui/pages/DashboardPage.h"

#include <QDate>
#include <QFileDialog>
#include <QFrame>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QVBoxLayout>

#include "data/Database.h"
#include "data/LoanRepository.h"
#include "services/StatsService.h"
#include "ui/common/Msg.h"
#include "ui/common/TableUtil.h"
#include "util/Jalali.h"
#include "util/TextUtil.h"

DashboardPage::DashboardPage(QWidget* parent) : Page(parent) {
    auto* title = new QLabel("داشبورد");
    title->setObjectName("PageTitle");
    date_ = new QLabel;
    date_->setObjectName("PageSubtitle");

    auto* titleBox = new QVBoxLayout;
    titleBox->setSpacing(2);
    titleBox->addWidget(title);
    titleBox->addWidget(date_);

    auto* backupButton = new QPushButton("پشتیبان‌گیری از بانک اطلاعات");
    connect(backupButton, &QPushButton::clicked, this, &DashboardPage::backup);

    auto* header = new QHBoxLayout;
    header->addLayout(titleBox);
    header->addStretch();
    header->addWidget(backupButton, 0, Qt::AlignTop);

    auto* cards = new QHBoxLayout;
    cards->setSpacing(14);
    cards->addWidget(makeCard("عنوان کتاب", &titles_, "StatValue"));
    cards->addWidget(makeCard("مجموع نسخه‌ها", &copies_, "StatValue"));
    cards->addWidget(makeCard("امانت‌های فعال", &active_, "StatValue"));
    cards->addWidget(makeCard("در تأخیر", &overdue_, "StatValueWarn"));
    cards->addWidget(makeCard("اعضا", &members_, "StatValue"));

    auto* recentTitle = new QLabel("آخرین امانت‌ها");
    recentTitle->setObjectName("SectionTitle");

    recent_ = TableUtil::create({"کتاب", "امانت‌گیرنده", "تاریخ امانت", "سررسید", "وضعیت"});
    recent_->horizontalHeader()->setSectionResizeMode(0, QHeaderView::Stretch);
    recent_->setSelectionMode(QAbstractItemView::NoSelection);

    auto* layout = new QVBoxLayout(this);
    layout->setContentsMargins(28, 24, 28, 20);
    layout->setSpacing(16);
    layout->addLayout(header);
    layout->addLayout(cards);
    layout->addWidget(recentTitle);
    layout->addWidget(recent_, 1);
}

QFrame* DashboardPage::makeCard(const QString& caption, QLabel** valueOut, const char* valueObjectName) {
    auto* card = new QFrame;
    card->setObjectName("Card");
    auto* lay = new QVBoxLayout(card);
    lay->setContentsMargins(18, 16, 18, 16);
    lay->setSpacing(4);

    auto* value = new QLabel("۰");
    value->setObjectName(valueObjectName);
    auto* cap = new QLabel(caption);
    cap->setObjectName("StatCaption");
    lay->addWidget(value);
    lay->addWidget(cap);

    *valueOut = value;
    return card;
}

void DashboardPage::refresh() {
    date_->setText(Jalali::longFormat(QDate::currentDate()));

    const Stats s = StatsService().compute();
    titles_->setText(TextUtil::fa(s.titles));
    copies_->setText(TextUtil::fa(s.copies));
    active_->setText(TextUtil::fa(s.activeLoans));
    overdue_->setText(TextUtil::fa(s.overdue));
    members_->setText(TextUtil::fa(s.members));

    const QList<Loan> loans = LoanRepository().list(LoanFilter::All, QString(), 10);
    recent_->setRowCount(0);
    recent_->setRowCount(loans.size());
    int row = 0;
    for (const Loan& l : loans) {
        recent_->setItem(row, 0, TableUtil::item(l.bookTitle));
        recent_->setItem(row, 1, TableUtil::item(l.memberName));
        recent_->setItem(row, 2, TableUtil::item(Jalali::format(l.loanDate), true));
        recent_->setItem(row, 3, TableUtil::item(Jalali::format(l.dueDate), true));

        QString status = "امانت فعال";
        QColor color("#1f6b57");
        if (l.isReturned()) {
            status = "بازگردانده شد";
            color = QColor("#6b7d76");
        } else if (l.isOverdue()) {
            status = "در تأخیر";
            color = QColor("#b3261e");
        }
        auto* statusItem = TableUtil::item(status, true);
        statusItem->setForeground(color);
        recent_->setItem(row, 4, statusItem);
        ++row;
    }
}

void DashboardPage::backup() {
    const QString suggested =
        "ganj-backup-" + QDate::currentDate().toString("yyyyMMdd") + ".db";
    const QString path =
        QFileDialog::getSaveFileName(this, "پشتیبان‌گیری", suggested, "Database (*.db)");
    if (path.isEmpty()) return;
    QString error;
    if (Database::backup(path, &error))
        Msg::info(this, "پشتیبان‌گیری", "فایل پشتیبان با موفقیت ذخیره شد.");
    else
        Msg::warn(this, "خطا", error);
}
