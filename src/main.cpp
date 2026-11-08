#include <QApplication>
#include <QEventLoop>
#include <QTimer>

#include "data/Database.h"
#include "ui/MainWindow.h"
#include "ui/SplashWidget.h"
#include "ui/Theme.h"
#include "ui/common/Msg.h"

namespace {

// Lets the splash screen repaint between start-up steps
void pause(int milliseconds) {
    QEventLoop loop;
    QTimer::singleShot(milliseconds, &loop, &QEventLoop::quit);
    loop.exec();
}

}  // namespace

int main(int argc, char* argv[]) {
    QApplication app(argc, argv);
    QCoreApplication::setOrganizationName("Ganj");
    QCoreApplication::setApplicationName("Ganj");
    QCoreApplication::setApplicationVersion("1.0.0");
    app.setLayoutDirection(Qt::RightToLeft);

    Theme::loadFonts();
    Theme::apply(app);

    SplashWidget splash;
    splash.show();
    splash.setProgress(8, "در حال راه‌اندازی...");
    pause(350);

    splash.setProgress(30, "اتصال به بانک اطلاعات...");
    QString error;
    if (!Database::open(&error)) {
        splash.close();
        Msg::warn(nullptr, "خطای بانک اطلاعات", "باز کردن بانک اطلاعات ممکن نشد:\n" + error);
        return 1;
    }
    pause(450);

    splash.setProgress(60, "بارگذاری اطلاعات کتابخانه...");
    pause(350);

    splash.setProgress(85, "آماده‌سازی رابط کاربری...");
    MainWindow window;
    pause(350);

    splash.setProgress(100, "آماده است");
    pause(300);

    window.show();
    splash.close();
    return app.exec();
}
