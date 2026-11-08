#pragma once
#include "data/BookRepository.h"
#include "ui/pages/Page.h"

class QLabel;
class QLineEdit;
class QTableWidget;
class QTimer;

class BooksPage : public Page {
    Q_OBJECT
public:
    explicit BooksPage(QWidget* parent = nullptr);
    void refresh() override;

private slots:
    void addBook();
    void editSelected();
    void deleteSelected();
    void importFromExcel();
    void saveTemplate();
    void showImportHelp();

private:
    BookRepository repo_;
    QLineEdit* search_;
    QTableWidget* table_;
    QLabel* count_;
    QTimer* debounce_;
};
