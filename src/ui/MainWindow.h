#pragma once
#include <QList>
#include <QMainWindow>

class QPushButton;
class QStackedWidget;

class MainWindow : public QMainWindow {
    Q_OBJECT
public:
    MainWindow();

private:
    void navigate(int index);

    QStackedWidget* stack_;
    QList<QPushButton*> navButtons_;
};
