#include "ui/MainWindow.h"

#include <QButtonGroup>
#include <QFrame>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QStackedWidget>
#include <QVBoxLayout>

#include "ui/pages/BooksPage.h"
#include "ui/pages/DashboardPage.h"
#include "ui/pages/LoansPage.h"
#include "ui/pages/MembersPage.h"

MainWindow::MainWindow() {
    setWindowTitle("سامانه مدیریت کتابخانه گنج");
    resize(1280, 800);
    setMinimumSize(1050, 650);

    auto* central = new QWidget;
    central->setObjectName("Content");
    auto* root = new QHBoxLayout(central);
    root->setContentsMargins(0, 0, 0, 0);
    root->setSpacing(0);

    // ---- Sidebar (first widget = right side because the layout is right-to-left) ----
    auto* sidebar = new QFrame;
    sidebar->setObjectName("Sidebar");
    sidebar->setFixedWidth(250);
    auto* side = new QVBoxLayout(sidebar);
    side->setContentsMargins(16, 28, 16, 16);
    side->setSpacing(6);

    auto* logo = new QLabel("گنج");
    logo->setObjectName("Logo");
    logo->setAlignment(Qt::AlignCenter);
    auto* logoSub = new QLabel("سامانه مدیریت کتابخانه");
    logoSub->setObjectName("LogoSub");
    logoSub->setAlignment(Qt::AlignCenter);
    side->addWidget(logo);
    side->addWidget(logoSub);
    side->addSpacing(28);

    stack_ = new QStackedWidget;
    auto* group = new QButtonGroup(this);
    group->setExclusive(true);

    auto addPage = [&](const QString& label, Page* page) {
        const int index = stack_->addWidget(page);
        auto* button = new QPushButton(label);
        button->setObjectName("NavButton");
        button->setCheckable(true);
        button->setCursor(Qt::PointingHandCursor);
        group->addButton(button);
        side->addWidget(button);
        navButtons_.append(button);
        connect(button, &QPushButton::clicked, this, [this, index] { navigate(index); });
    };
    addPage("📊   داشبورد", new DashboardPage);
    addPage("📚   کتاب‌ها", new BooksPage);
    addPage("👥   اعضا", new MembersPage);
    addPage("🔄   امانت و بازگشت", new LoansPage);

    side->addStretch();
    auto* footer = new QLabel("تیپ ۱۱۶ پیاده مکانیزه\nشهید سرلشگر صفوی");
    footer->setObjectName("SidebarFooter");
    footer->setAlignment(Qt::AlignCenter);
    side->addWidget(footer);

    root->addWidget(sidebar);
    root->addWidget(stack_, 1);
    setCentralWidget(central);

    navigate(0);
}

void MainWindow::navigate(int index) {
    stack_->setCurrentIndex(index);
    navButtons_[index]->setChecked(true);
    if (auto* page = qobject_cast<Page*>(stack_->currentWidget())) page->refresh();
}
