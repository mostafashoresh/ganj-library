#pragma once
#include <QWidget>

// Base class of every screen shown in the main window.
class Page : public QWidget {
    Q_OBJECT
public:
    explicit Page(QWidget* parent = nullptr) : QWidget(parent) {}
    virtual void refresh() = 0;   // reload data from the database
};
