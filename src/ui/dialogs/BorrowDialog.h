#pragma once
#include <QDate>
#include <QDialog>

class QComboBox;
class QLabel;
class QSpinBox;

class BorrowDialog : public QDialog {
    Q_OBJECT
public:
    explicit BorrowDialog(QWidget* parent = nullptr);
    qint64 bookId() const { return bookId_; }
    qint64 memberId() const { return memberId_; }
    QDate dueDate() const;

protected:
    void accept() override;

private slots:
    void addMember();
    void updateDueLabel();

private:
    void loadBooks();
    void loadMembers(qint64 selectId = 0);
    static void makeSearchable(QComboBox* combo, const QString& placeholder);

    QComboBox *book_, *member_;
    QSpinBox* days_;
    QLabel* dueLabel_;
    qint64 bookId_ = 0;
    qint64 memberId_ = 0;
};
