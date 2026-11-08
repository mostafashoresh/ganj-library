#include "ui/pages/BooksPage.h"

#include <QFileDialog>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QTimer>
#include <QVBoxLayout>

#include "services/ExcelImporter.h"
#include "ui/common/Msg.h"
#include "ui/common/TableUtil.h"
#include "ui/dialogs/BookDialog.h"
#include "util/TextUtil.h"

BooksPage::BooksPage(QWidget* parent) : Page(parent) {
    auto* title = new QLabel("کتاب‌ها");
    title->setObjectName("PageTitle");

    search_ = new QLineEdit;
    search_->setPlaceholderText("جستجو در نام کتاب، نویسنده، ناشر، شابک، دسته‌بندی، قفسه...");
    search_->setClearButtonEnabled(true);

    auto* addButton = new QPushButton("＋ افزودن کتاب");
    addButton->setObjectName("Primary");
    auto* importButton = new QPushButton("ورود از اکسل");
    auto* helpButton = new QPushButton("؟");
    helpButton->setFixedWidth(40);
    helpButton->setToolTip("راهنمای فرمت فایل اکسل");
    auto* editButton = new QPushButton("ویرایش");
    auto* deleteButton = new QPushButton("حذف");
    deleteButton->setObjectName("Danger");

    auto* bar = new QHBoxLayout;
    bar->setSpacing(8);
    bar->addWidget(search_, 1);
    bar->addWidget(addButton);
    bar->addWidget(importButton);
    bar->addWidget(helpButton);
    bar->addWidget(editButton);
    bar->addWidget(deleteButton);

    table_ = TableUtil::create({"نام کتاب", "نویسنده", "ناشر", "شابک", "سال", "دسته‌بندی", "قفسه",
                                "تعداد", "موجود"});
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
    connect(debounce_, &QTimer::timeout, this, &BooksPage::refresh);
    connect(addButton, &QPushButton::clicked, this, &BooksPage::addBook);
    connect(editButton, &QPushButton::clicked, this, &BooksPage::editSelected);
    connect(deleteButton, &QPushButton::clicked, this, &BooksPage::deleteSelected);
    connect(importButton, &QPushButton::clicked, this, &BooksPage::importFromExcel);
    connect(helpButton, &QPushButton::clicked, this, &BooksPage::showImportHelp);
    connect(table_, &QTableWidget::cellDoubleClicked, this, [this] { editSelected(); });
}

void BooksPage::refresh() {
    const QList<Book> books = repo_.search(search_->text());
    table_->setRowCount(0);
    table_->setRowCount(books.size());

    int row = 0;
    for (const Book& b : books) {
        auto* titleItem = TableUtil::item(b.title);
        titleItem->setData(Qt::UserRole, b.id);
        table_->setItem(row, 0, titleItem);
        table_->setItem(row, 1, TableUtil::item(b.author));
        table_->setItem(row, 2, TableUtil::item(b.publisher));
        table_->setItem(row, 3, TableUtil::item(b.isbn));
        table_->setItem(row, 4, TableUtil::item(TextUtil::toPersianDigits(b.year), true));
        table_->setItem(row, 5, TableUtil::item(b.category));
        table_->setItem(row, 6, TableUtil::item(b.location, true));
        table_->setItem(row, 7, TableUtil::item(TextUtil::fa(b.copies), true));
        auto* availableItem = TableUtil::item(TextUtil::fa(b.available), true);
        if (b.available <= 0) availableItem->setForeground(QColor("#b3261e"));
        table_->setItem(row, 8, availableItem);
        ++row;
    }
    count_->setText(TextUtil::fa(books.size()) + " عنوان کتاب");
}

void BooksPage::addBook() {
    BookDialog dlg(this);
    if (dlg.exec() != QDialog::Accepted) return;
    Book b = dlg.book();
    QString error;
    if (!repo_.add(b, &error)) {
        Msg::warn(this, "خطا", error);
        return;
    }
    refresh();
}

void BooksPage::editSelected() {
    const qint64 id = TableUtil::currentId(table_);
    if (!id) {
        Msg::info(this, "ویرایش", "ابتدا یک کتاب را از لیست انتخاب کنید.");
        return;
    }
    Book current;
    for (const Book& b : repo_.search())
        if (b.id == id) current = b;

    BookDialog dlg(this, &current);
    if (dlg.exec() != QDialog::Accepted) return;
    QString error;
    if (!repo_.update(dlg.book(), &error)) {
        Msg::warn(this, "خطا", error);
        return;
    }
    refresh();
}

void BooksPage::deleteSelected() {
    const qint64 id = TableUtil::currentId(table_);
    if (!id) {
        Msg::info(this, "حذف", "ابتدا یک کتاب را از لیست انتخاب کنید.");
        return;
    }
    const QString name = table_->item(table_->currentRow(), 0)->text();
    if (!Msg::confirm(this, "حذف کتاب", "کتاب «" + name + "» حذف شود؟", "حذف", "انصراف")) return;
    QString error;
    if (!repo_.remove(id, &error)) {
        Msg::warn(this, "خطا", error);
        return;
    }
    refresh();
}

void BooksPage::importFromExcel() {
    const QString path = QFileDialog::getOpenFileName(
        this, "انتخاب فایل اکسل", QString(), "Excel / CSV (*.xlsx *.csv)");
    if (path.isEmpty()) return;

    const ImportResult result = ExcelImporter::readBooks(path);
    if (!result.error.isEmpty()) {
        Msg::warn(this, "خطا در خواندن فایل", result.error);
        return;
    }
    if (result.books.isEmpty()) {
        Msg::warn(this, "ورود از اکسل",
                  "هیچ کتابی با «نام کتاب» در فایل پیدا نشد.\nبرای دیدن فرمت درست، دکمه‌ی «؟» را بزنید.");
        return;
    }

    QString question = TextUtil::fa(result.books.size()) + " کتاب در فایل پیدا شد.";
    if (result.skippedNoTitle > 0)
        question += "\n" + TextUtil::fa(result.skippedNoTitle) + " ردیف بدون نام کتاب نادیده گرفته می‌شود.";
    question += "\n\nکتاب‌های تکراری (هم‌نام و هم‌نویسنده) اضافه نمی‌شوند. ادامه می‌دهید؟";
    if (!Msg::confirm(this, "ورود از اکسل", question, "افزودن", "انصراف")) return;

    const ImportSummary s = repo_.addMany(result.books);
    refresh();

    QString report = TextUtil::fa(s.added) + " کتاب اضافه شد.";
    if (s.duplicates) report += "\n" + TextUtil::fa(s.duplicates) + " کتاب تکراری بود و نادیده گرفته شد.";
    if (s.failed) report += "\n" + TextUtil::fa(s.failed) + " ردیف به‌خاطر خطا اضافه نشد.";
    Msg::info(this, "نتیجه‌ی ورود اطلاعات", report);
}

void BooksPage::saveTemplate() {
    const QString path = QFileDialog::getSaveFileName(this, "ذخیره‌ی فایل نمونه", "books-template.csv",
                                                      "CSV (*.csv)");
    if (path.isEmpty()) return;
    QString error;
    if (!ExcelImporter::writeCsvTemplate(path, &error)) Msg::warn(this, "خطا", error);
}

void BooksPage::showImportHelp() {
    const QString text =
        "فایل اکسل (xlsx) یا CSV با این ستون‌ها (ردیف اول = عنوان ستون‌ها):\n\n"
        "نام کتاب (الزامی) | نویسنده | ناشر | شابک | سال انتشار | دسته‌بندی | قفسه | تعداد\n\n"
        "• فقط «نام کتاب» الزامی است؛ بقیه‌ی ستون‌ها اختیاری‌اند و ترتیبشان مهم نیست.\n"
        "• اگر ردیف عنوان نداشته باشید، ستون‌ها به همین ترتیب خوانده می‌شوند.\n"
        "• ردیف‌هایی که نام کتاب ندارند نادیده گرفته می‌شوند.\n\n"
        "می‌خواهید یک فایل نمونه ذخیره کنید؟";
    if (Msg::confirm(this, "راهنمای فایل اکسل", text, "ذخیره‌ی فایل نمونه", "بستن")) saveTemplate();
}
