#pragma once
#include <QString>

class QWidget;

// Persian-labelled message boxes
namespace Msg {
void info(QWidget* parent, const QString& title, const QString& text);
void warn(QWidget* parent, const QString& title, const QString& text);
bool confirm(QWidget* parent, const QString& title, const QString& text,
             const QString& yes = "بله", const QString& no = "خیر");
}  // namespace Msg
