#pragma once
#include "data/LoanRepository.h"
#include "ui/pages/Page.h"

class QComboBox;
class QLabel;
class QLineEdit;
class QTableWidget;
class QTimer;

class LoansPage : public Page {
    Q_OBJECT
public:
    explicit LoansPage(QWidget* parent = nullptr);
    void refresh() override;

private slots:
    void newLoan();
    void returnSelected();

private:
    LoanRepository repo_;
    QLineEdit* search_;
    QComboBox* filter_;
    QTableWidget* table_;
    QLabel* count_;
    QTimer* debounce_;
};
