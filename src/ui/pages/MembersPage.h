#pragma once
#include "data/MemberRepository.h"
#include "ui/pages/Page.h"

class QLabel;
class QLineEdit;
class QTableWidget;
class QTimer;

class MembersPage : public Page {
    Q_OBJECT
public:
    explicit MembersPage(QWidget* parent = nullptr);
    void refresh() override;

private slots:
    void addMember();
    void editSelected();
    void deleteSelected();

private:
    MemberRepository repo_;
    QLineEdit* search_;
    QTableWidget* table_;
    QLabel* count_;
    QTimer* debounce_;
};
