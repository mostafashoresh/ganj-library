#pragma once
#include <QDialog>

#include "core/Models.h"

class QLineEdit;
class QSpinBox;

class BookDialog : public QDialog {
    Q_OBJECT
public:
    explicit BookDialog(QWidget* parent = nullptr, const Book* existing = nullptr);
    Book book() const;

protected:
    void accept() override;

private:
    qint64 id_ = 0;
    QLineEdit *title_, *author_, *publisher_, *isbn_, *year_, *category_, *location_;
    QSpinBox* copies_;
};
