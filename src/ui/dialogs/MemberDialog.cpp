#include "ui/dialogs/MemberDialog.h"

#include <QDialogButtonBox>
#include <QFormLayout>
#include <QLineEdit>
#include <QPushButton>
#include <QVBoxLayout>

#include "ui/common/Msg.h"

MemberDialog::MemberDialog(QWidget* parent, const Member* existing) : QDialog(parent) {
    setWindowTitle(existing ? "ویرایش عضو" : "افزودن عضو");
    setMinimumWidth(440);

    name_ = new QLineEdit;
    name_->setPlaceholderText("الزامی");
    code_ = new QLineEdit;
    unit_ = new QLineEdit;
    phone_ = new QLineEdit;

    auto* form = new QFormLayout;
    form->setSpacing(12);
    form->addRow("نام و نام خانوادگی *", name_);
    form->addRow("کد پرسنلی", code_);
    form->addRow("یگان / واحد", unit_);
    form->addRow("تلفن", phone_);

    auto* buttons = new QDialogButtonBox;
    auto* ok = buttons->addButton("ذخیره", QDialogButtonBox::AcceptRole);
    ok->setObjectName("Primary");
    buttons->addButton("انصراف", QDialogButtonBox::RejectRole);
    connect(buttons, &QDialogButtonBox::accepted, this, &QDialog::accept);
    connect(buttons, &QDialogButtonBox::rejected, this, &QDialog::reject);

    auto* layout = new QVBoxLayout(this);
    layout->setContentsMargins(24, 20, 24, 20);
    layout->setSpacing(14);
    layout->addLayout(form);
    layout->addWidget(buttons);

    if (existing) {
        id_ = existing->id;
        name_->setText(existing->fullName);
        code_->setText(existing->personnelCode);
        unit_->setText(existing->unit);
        phone_->setText(existing->phone);
    }
    name_->setFocus();
}

Member MemberDialog::member() const {
    Member m;
    m.id = id_;
    m.fullName = name_->text();
    m.personnelCode = code_->text();
    m.unit = unit_->text();
    m.phone = phone_->text();
    return m;
}

void MemberDialog::accept() {
    if (name_->text().simplified().isEmpty()) {
        Msg::warn(this, "نام عضو", "وارد کردن نام الزامی است.");
        name_->setFocus();
        return;
    }
    QDialog::accept();
}
