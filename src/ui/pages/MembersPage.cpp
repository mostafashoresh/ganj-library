#include "ui/pages/MembersPage.h"

#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QTimer>
#include <QVBoxLayout>

#include "ui/common/Msg.h"
#include "ui/common/TableUtil.h"
#include "ui/dialogs/MemberDialog.h"
#include "util/TextUtil.h"

MembersPage::MembersPage(QWidget* parent) : Page(parent) {
    auto* title = new QLabel("اعضا");
    title->setObjectName("PageTitle");

    search_ = new QLineEdit;
    search_->setPlaceholderText("جستجو در نام، کد پرسنلی، یگان یا تلفن...");
    search_->setClearButtonEnabled(true);

    auto* addButton = new QPushButton("＋ افزودن عضو");
    addButton->setObjectName("Primary");
    auto* editButton = new QPushButton("ویرایش");
    auto* deleteButton = new QPushButton("حذف");
    deleteButton->setObjectName("Danger");

    auto* bar = new QHBoxLayout;
    bar->setSpacing(8);
    bar->addWidget(search_, 1);
    bar->addWidget(addButton);
    bar->addWidget(editButton);
    bar->addWidget(deleteButton);

    table_ = TableUtil::create({"نام و نام خانوادگی", "کد پرسنلی", "یگان / واحد", "تلفن", "امانت‌های فعال"});
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
    connect(debounce_, &QTimer::timeout, this, &MembersPage::refresh);
    connect(addButton, &QPushButton::clicked, this, &MembersPage::addMember);
    connect(editButton, &QPushButton::clicked, this, &MembersPage::editSelected);
    connect(deleteButton, &QPushButton::clicked, this, &MembersPage::deleteSelected);
    connect(table_, &QTableWidget::cellDoubleClicked, this, [this] { editSelected(); });
}

void MembersPage::refresh() {
    const QList<Member> members = repo_.search(search_->text());
    table_->setRowCount(0);
    table_->setRowCount(members.size());

    int row = 0;
    for (const Member& m : members) {
        auto* nameItem = TableUtil::item(m.fullName);
        nameItem->setData(Qt::UserRole, m.id);
        table_->setItem(row, 0, nameItem);
        table_->setItem(row, 1, TableUtil::item(m.personnelCode, true));
        table_->setItem(row, 2, TableUtil::item(m.unit));
        table_->setItem(row, 3, TableUtil::item(m.phone, true));
        table_->setItem(row, 4, TableUtil::item(TextUtil::fa(m.activeLoans), true));
        ++row;
    }
    count_->setText(TextUtil::fa(members.size()) + " عضو");
}

void MembersPage::addMember() {
    MemberDialog dlg(this);
    if (dlg.exec() != QDialog::Accepted) return;
    Member m = dlg.member();
    QString error;
    if (!repo_.add(m, &error)) {
        Msg::warn(this, "خطا", error);
        return;
    }
    refresh();
}

void MembersPage::editSelected() {
    const qint64 id = TableUtil::currentId(table_);
    if (!id) {
        Msg::info(this, "ویرایش", "ابتدا یک عضو را از لیست انتخاب کنید.");
        return;
    }
    Member current;
    for (const Member& m : repo_.search())
        if (m.id == id) current = m;

    MemberDialog dlg(this, &current);
    if (dlg.exec() != QDialog::Accepted) return;
    QString error;
    if (!repo_.update(dlg.member(), &error)) {
        Msg::warn(this, "خطا", error);
        return;
    }
    refresh();
}

void MembersPage::deleteSelected() {
    const qint64 id = TableUtil::currentId(table_);
    if (!id) {
        Msg::info(this, "حذف", "ابتدا یک عضو را از لیست انتخاب کنید.");
        return;
    }
    const QString name = table_->item(table_->currentRow(), 0)->text();
    if (!Msg::confirm(this, "حذف عضو", "عضو «" + name + "» و سوابق امانت او حذف شود؟", "حذف", "انصراف"))
        return;
    QString error;
    if (!repo_.remove(id, &error)) {
        Msg::warn(this, "خطا", error);
        return;
    }
    refresh();
}
