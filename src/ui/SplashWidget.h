#pragma once
#include <QString>
#include <QWidget>

// Start-up loader: system name, unit name and a progress bar.
class SplashWidget : public QWidget {
    Q_OBJECT
public:
    SplashWidget();
    void setProgress(int percent, const QString& message);

protected:
    void paintEvent(QPaintEvent* event) override;

private:
    int percent_ = 0;
    QString message_;
};
