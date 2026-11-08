#include "ui/pages/LoansPage.h"

#include <QComboBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QTimer>
#include <QVBoxLayout>

#include "ui/common/Msg.h"
#include "ui/common/TableUtil.h"
#include "ui/dialogs/BorrowDialog.h"
#include "util/Jalali.h"
#include "util/TextUtil.h"

LoansPage::LoansPage(QWidget* parent) : Page(parent) {
    auto* title = new QLabel("امانت و بازگشت");
    title->setObjectName("PageTitle");

    search_ = new QLineEdit;
    search_->setPlaceholderText("جستجو در نام کتاب یا امانت‌گیرنده...");
    search_->setClearButtonEnabled(true);

    filter_ = new QComboBox;
    filter_->addItem("امانت‌های فعال", static_cast<int>(LoanFilter::Active));
    filter_->addItem("در تأخیر", static_cast<int>(LoanFilter::Overdue));
    filter_->addItem("بازگردانده‌شده", static_cast<int>(LoanFilter::Returned));
    filter_->addItem("همه", static_cast<int>(LoanFilter::All));

    auto* newButton = new QPushButton("＋ امانت جدید");
    newButton->setObjectName("Primary");
    auto* returnButton = new QPushButton("ثبت بازگشت کتاب");

    auto* bar = new QHBoxLayout;
    bar->setSpacing(8);
    bar->addWidget(search_, 1);
    bar->addWidget(filter_);
    bar->addWidget(newButton);
    bar->addWidget(returnButton);

    table_ = TableUtil::create({"کتاب", "امانت‌گیرنده", "تاریخ امانت", "سررسید", "تاریخ بازگشت", "وضعیت"});
    table_->horizontalHeader()->setSectionResizeMode(0, QHeaderView::Stretch);

    count_ = new QLabel;
    count_->setObjectName("PageSubtitle");

    auto* layout = new QVBoxLayout(this);
    layout->setContentsMargins(28, 24, 28, 20);
    layout->setSpacing(14);
    layout->addWidget(title);
    layout->addLayout(bar);
    layout->addWidget(table_, 1);
    layout->addWidget(count_);

    debounce_ = new QTimer(this);
    debounce_->setSingleShot(true);
    debounce_->setInterval(250);

    connect(search_, &QLineEdit::textChanged, debounce_, qOverload<>(&QTimer::start));
    connect(debounce_, &QTimer::timeout, this, &LoansPage::refresh);
    connect(filter_, qOverload<int>(&QComboBox::currentIndexChanged), this, [this] { refresh(); });
    connect(newButton, &QPushButton::clicked, this, &LoansPage::newLoan);
    connect(returnButton, &QPushButton::clicked, this, &LoansPage::returnSelected);
}

void LoansPage::refresh() {
    const auto filter = static_cast<LoanFilter>(filter_->currentData().toInt());
    const QList<Loan> loans = repo_.list(filter, search_->text());

    table_->setRowCount(0);
    table_->setRowCount(loans.size());
    int row = 0;
    for (const Loan& l : loans) {
        auto* bookItem = TableUtil::item(l.bookTitle);
        bookItem->setData(Qt::UserRole, l.id);
        bookItem->setData(Qt::UserRole + 1, l.isReturned());
        table_->setItem(row, 0, bookItem);
        table_->setItem(row, 1, TableUtil::item(l.memberName));
        table_->setItem(row, 2, TableUtil::item(Jalali::format(l.loanDate), true));
        table_->setItem(row, 3, TableUtil::item(Jalali::format(l.dueDate), true));
        table_->setItem(row, 4, TableUtil::item(l.isReturned() ? Jalali::format(l.returnDate) : "—", true));

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
        table_->setItem(row, 5, statusItem);
        ++row;
    }
    count_->setText(TextUtil::fa(loans.size()) + " مورد");
}

void LoansPage::newLoan() {
    BorrowDialog dlg(this);
    if (dlg.exec() != QDialog::Accepted) return;
    QString error;
    if (!repo_.borrow(dlg.bookId(), dlg.memberId(), dlg.dueDate(), &error)) {
        Msg::warn(this, "خطا", error);
        return;
    }
    refresh();
}

void LoansPage::returnSelected() {
    const qint64 id = TableUtil::currentId(table_);
    if (!id) {
        Msg::info(this, "بازگشت کتاب", "ابتدا یک امانت را از لیست انتخاب کنید.");
        return;
    }
    if (table_->item(table_->currentRow(), 0)->data(Qt::UserRole + 1).toBool()) {
        Msg::info(this, "بازگشت کتاب", "این کتاب قبلاً بازگردانده شده است.");
        return;
    }
    const QString name = table_->item(table_->currentRow(), 0)->text();
    if (!Msg::confirm(this, "بازگشت کتاب", "بازگشت کتاب «" + name + "» ثبت شود؟", "ثبت بازگشت", "انصراف"))
        return;
    QString error;
    if (!repo_.giveBack(id, &error)) {
        Msg::warn(this, "خطا", error);
        return;
    }
    refresh();
}
