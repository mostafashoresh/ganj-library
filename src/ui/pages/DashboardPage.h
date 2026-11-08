#pragma once
#include "ui/pages/Page.h"

class QFrame;
class QLabel;
class QTableWidget;

class DashboardPage : public Page {
    Q_OBJECT
public:
    explicit DashboardPage(QWidget* parent = nullptr);
    void refresh() override;

private slots:
    void backup();

private:
    QFrame* makeCard(const QString& caption, QLabel** valueOut, const char* valueObjectName);

    QLabel *titles_, *copies_, *active_, *members_, *overdue_, *date_;
    QTableWidget* recent_;
};
