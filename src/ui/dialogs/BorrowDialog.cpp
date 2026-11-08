#include "ui/dialogs/BorrowDialog.h"

#include <QComboBox>
#include <QCompleter>
#include <QDialogButtonBox>
#include <QFormLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QSpinBox>
#include <QVBoxLayout>

#include "data/BookRepository.h"
#include "data/MemberRepository.h"
#include "ui/common/Msg.h"
#include "ui/dialogs/MemberDialog.h"
#include "util/Jalali.h"

BorrowDialog::BorrowDialog(QWidget* parent) : QDialog(parent) {
    setWindowTitle("ثبت امانت جدید");
    setMinimumWidth(560);

    book_ = new QComboBox;
    member_ = new QComboBox;
    makeSearchable(book_, "نام کتاب را تایپ یا انتخاب کنید...");
    makeSearchable(member_, "نام عضو را تایپ یا انتخاب کنید...");

    auto* newMember = new QPushButton("+ عضو جدید");
    auto* memberRow = new QHBoxLayout;
    memberRow->addWidget(member_, 1);
    memberRow->addWidget(newMember);

    days_ = new QSpinBox;
    days_->setRange(1, 365);
    days_->setValue(14);
    days_->setSuffix(" روز");
    dueLabel_ = new QLabel;
    dueLabel_->setObjectName("PageSubtitle");

    auto* form = new QFormLayout;
    form->setSpacing(12);
    form->addRow("کتاب", book_);
    form->addRow("امانت‌گیرنده", memberRow);
    form->addRow("مدت امانت", days_);
    form->addRow("", dueLabel_);

    auto* buttons = new QDialogButtonBox;
    auto* ok = buttons->addButton("ثبت امانت", QDialogButtonBox::AcceptRole);
    ok->setObjectName("Primary");
    buttons->addButton("انصراف", QDialogButtonBox::RejectRole);
    connect(buttons, &QDialogButtonBox::accepted, this, &QDialog::accept);
    connect(buttons, &QDialogButtonBox::rejected, this, &QDialog::reject);

    auto* layout = new QVBoxLayout(this);
    layout->setContentsMargins(24, 20, 24, 20);
    layout->setSpacing(14);
    layout->addLayout(form);
    layout->addWidget(buttons);

    connect(newMember, &QPushButton::clicked, this, &BorrowDialog::addMember);
    connect(days_, qOverload<int>(&QSpinBox::valueChanged), this, &BorrowDialog::updateDueLabel);

    loadBooks();
    loadMembers();
    updateDueLabel();
}

void BorrowDialog::makeSearchable(QComboBox* combo, const QString& placeholder) {
    combo->setEditable(true);
    combo->setInsertPolicy(QComboBox::NoInsert);
    combo->completer()->setFilterMode(Qt::MatchContains);
    combo->completer()->setCaseSensitivity(Qt::CaseInsensitive);
    combo->lineEdit()->setPlaceholderText(placeholder);
}

void BorrowDialog::loadBooks() {
    book_->clear();
    for (const Book& b : BookRepository().search()) {
        if (b.available <= 0) continue;   // only books that can be borrowed
        QString label = b.title;
        if (!b.author.isEmpty()) label += "  —  " + b.author;
        book_->addItem(label, b.id);
    }
    book_->setCurrentIndex(-1);
}

void BorrowDialog::loadMembers(qint64 selectId) {
    member_->clear();
    for (const Member& m : MemberRepository().search()) {
        QString label = m.fullName;
        if (!m.personnelCode.isEmpty()) label += "  (" + m.personnelCode + ")";
        member_->addItem(label, m.id);
    }
    member_->setCurrentIndex(selectId ? member_->findData(selectId) : -1);
}

void BorrowDialog::addMember() {
    MemberDialog dlg(this);
    if (dlg.exec() != QDialog::Accepted) return;
    Member m = dlg.member();
    QString error;
    if (!MemberRepository().add(m, &error)) {
        Msg::warn(this, "خطا", error);
        return;
    }
    loadMembers(m.id);
}

QDate BorrowDialog::dueDate() const {
    return QDate::currentDate().addDays(days_->value());
}

void BorrowDialog::updateDueLabel() {
    dueLabel_->setText("سررسید بازگشت: " + Jalali::format(dueDate()));
}

void BorrowDialog::accept() {
    const int bi = book_->findText(book_->currentText(), Qt::MatchFixedString);
    if (bi < 0) {
        Msg::warn(this, "کتاب", "یک کتاب موجود را از لیست انتخاب کنید.");
        return;
    }
    const int mi = member_->findText(member_->currentText(), Qt::MatchFixedString);
    if (mi < 0) {
        Msg::warn(this, "امانت‌گیرنده", "یک عضو را از لیست انتخاب کنید یا عضو جدید بسازید.");
        return;
    }
    bookId_ = book_->itemData(bi).toLongLong();
    memberId_ = member_->itemData(mi).toLongLong();
    QDialog::accept();
}
