#pragma once
#include <QDialog>

#include "core/Models.h"

class QLineEdit;

class MemberDialog : public QDialog {
    Q_OBJECT
public:
    explicit MemberDialog(QWidget* parent = nullptr, const Member* existing = nullptr);
    Member member() const;

protected:
    void accept() override;

private:
    qint64 id_ = 0;
    QLineEdit *name_, *code_, *unit_, *phone_;
};
