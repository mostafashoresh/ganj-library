#include "ui/dialogs/BookDialog.h"

#include <QDialogButtonBox>
#include <QFormLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QSpinBox>
#include <QVBoxLayout>

#include "ui/common/Msg.h"

BookDialog::BookDialog(QWidget* parent, const Book* existing) : QDialog(parent) {
    setWindowTitle(existing ? "ویرایش کتاب" : "افزودن کتاب");
    setMinimumWidth(480);

    title_ = new QLineEdit;
    title_->setPlaceholderText("الزامی");
    author_ = new QLineEdit;
    publisher_ = new QLineEdit;
    isbn_ = new QLineEdit;
    year_ = new QLineEdit;
    category_ = new QLineEdit;
    location_ = new QLineEdit;
    copies_ = new QSpinBox;
    copies_->setRange(1, 9999);

    auto* form = new QFormLayout;
    form->setSpacing(12);
    form->addRow("نام کتاب *", title_);
    form->addRow("نویسنده", author_);
    form->addRow("ناشر", publisher_);
    form->addRow("شابک", isbn_);
    form->addRow("سال انتشار", year_);
    form->addRow("دسته‌بندی", category_);
    form->addRow("قفسه / محل", location_);
    form->addRow("تعداد نسخه", copies_);

    auto* hint = new QLabel("فقط «نام کتاب» الزامی است؛ بقیه‌ی فیلدها را می‌توانید خالی بگذارید.");
    hint->setObjectName("PageSubtitle");
    hint->setWordWrap(true);

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
    layout->addWidget(hint);
    layout->addWidget(buttons);

    if (existing) {
        id_ = existing->id;
        title_->setText(existing->title);
        author_->setText(existing->author);
        publisher_->setText(existing->publisher);
        isbn_->setText(existing->isbn);
        year_->setText(existing->year);
        category_->setText(existing->category);
        location_->setText(existing->location);
        copies_->setValue(existing->copies);
    }
    title_->setFocus();
}

Book BookDialog::book() const {
    Book b;
    b.id = id_;
    b.title = title_->text();
    b.author = author_->text();
    b.publisher = publisher_->text();
    b.isbn = isbn_->text();
    b.year = year_->text();
    b.category = category_->text();
    b.location = location_->text();
    b.copies = copies_->value();
    return b;
}

void BookDialog::accept() {
    if (title_->text().simplified().isEmpty()) {
        Msg::warn(this, "نام کتاب", "وارد کردن نام کتاب الزامی است.");
        title_->setFocus();
        return;
    }
    QDialog::accept();
}
