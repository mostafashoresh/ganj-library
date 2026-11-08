#include "ui/Theme.h"

#include <QApplication>
#include <QCoreApplication>
#include <QDir>
#include <QFile>
#include <QFont>
#include <QFontDatabase>

namespace Theme {

void loadFonts() {
    QDir dir(QCoreApplication::applicationDirPath() + "/fonts");
    const QStringList files = dir.entryList({"*.ttf", "*.otf"}, QDir::Files);
    for (const QString& file : files)
        QFontDatabase::addApplicationFont(dir.absoluteFilePath(file));
}

void apply(QApplication& app) {
    app.setStyle("Fusion");

    QFile qss(":/style.qss");
    if (qss.open(QIODevice::ReadOnly)) app.setStyleSheet(QString::fromUtf8(qss.readAll()));

    QFont font("Segoe UI", 10);
    font.setFamilies({"Vazirmatn", "Segoe UI", "Tahoma"});
    app.setFont(font);
}

}  // namespace Theme
