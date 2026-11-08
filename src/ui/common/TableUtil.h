#pragma once
#include <QAbstractItemView>
#include <QHeaderView>
#include <QStringList>
#include <QTableWidget>
#include <QTableWidgetItem>

namespace TableUtil {

inline QTableWidget* create(const QStringList& headers) {
    auto* t = new QTableWidget(0, headers.size());
    t->setHorizontalHeaderLabels(headers);
    t->setSelectionBehavior(QAbstractItemView::SelectRows);
    t->setSelectionMode(QAbstractItemView::SingleSelection);
    t->setEditTriggers(QAbstractItemView::NoEditTriggers);
    t->setAlternatingRowColors(true);
    t->setShowGrid(false);
    t->verticalHeader()->setVisible(false);
    t->verticalHeader()->setDefaultSectionSize(42);
    t->horizontalHeader()->setHighlightSections(false);
    t->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
    t->horizontalHeader()->setMinimumSectionSize(90);
    return t;
}

inline QTableWidgetItem* item(const QString& text, bool center = false) {
    auto* it = new QTableWidgetItem(text);
    if (center) it->setTextAlignment(Qt::AlignCenter);
    return it;
}

// The first column's item carries the record id in Qt::UserRole
inline qint64 currentId(const QTableWidget* t) {
    const int row = t->currentRow();
    if (row < 0 || !t->item(row, 0)) return 0;
    return t->item(row, 0)->data(Qt::UserRole).toLongLong();
}

}  // namespace TableUtil
