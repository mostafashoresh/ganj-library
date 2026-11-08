#include "ui/common/Msg.h"

#include <QMessageBox>
#include <QPushButton>

namespace {

void show(QWidget* parent, QMessageBox::Icon icon, const QString& title, const QString& text) {
    QMessageBox box(icon, title, text, QMessageBox::NoButton, parent);
    box.addButton("باشه", QMessageBox::AcceptRole);
    box.exec();
}

}  // namespace

namespace Msg {

void info(QWidget* parent, const QString& title, const QString& text) {
    show(parent, QMessageBox::Information, title, text);
}

void warn(QWidget* parent, const QString& title, const QString& text) {
    show(parent, QMessageBox::Warning, title, text);
}

bool confirm(QWidget* parent, const QString& title, const QString& text, const QString& yes,
             const QString& no) {
    QMessageBox box(QMessageBox::Question, title, text, QMessageBox::NoButton, parent);
    QPushButton* yesButton = box.addButton(yes, QMessageBox::AcceptRole);
    box.addButton(no, QMessageBox::RejectRole);
    box.exec();
    return box.clickedButton() == yesButton;
}

}  // namespace Msg
