#include "ui/SplashWidget.h"

#include <QCoreApplication>
#include <QGuiApplication>
#include <QLinearGradient>
#include <QPainter>
#include <QPainterPath>
#include <QScreen>
#include <QTransform>

SplashWidget::SplashWidget() {
    setWindowFlags(Qt::SplashScreen | Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint);
    setAttribute(Qt::WA_TranslucentBackground);
    setFixedSize(700, 420);
    if (QScreen* screen = QGuiApplication::primaryScreen())
        move(screen->geometry().center() - rect().center());
}

void SplashWidget::setProgress(int percent, const QString& message) {
    percent_ = qBound(0, percent, 100);
    message_ = message;
    repaint();
    QCoreApplication::processEvents();
}

void SplashWidget::paintEvent(QPaintEvent*) {
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing);
    p.setRenderHint(QPainter::TextAntialiasing);

    const QColor gold("#e3c55b");

    // Background card
    QPainterPath card;
    card.addRoundedRect(QRectF(rect()).adjusted(6, 6, -6, -6), 24, 24);
    QLinearGradient bg(0, 0, 0, height());
    bg.setColorAt(0.0, QColor("#0d2a22"));
    bg.setColorAt(1.0, QColor("#17443a"));
    p.fillPath(card, bg);
    p.setPen(QPen(gold, 2));
    p.drawPath(card);

    // Emblem: ring + open book
    const QPointF c(width() / 2.0, 96);
    p.setPen(QPen(gold, 3));
    p.setBrush(QColor(255, 255, 255, 14));
    p.drawEllipse(c, 48, 48);

    QPainterPath left;
    left.moveTo(c.x(), c.y() + 24);
    left.cubicTo(c.x() - 10, c.y() + 15, c.x() - 26, c.y() + 13, c.x() - 36, c.y() + 18);
    left.lineTo(c.x() - 36, c.y() - 20);
    left.cubicTo(c.x() - 26, c.y() - 25, c.x() - 10, c.y() - 23, c.x(), c.y() - 14);
    left.closeSubpath();
    QTransform mirror;
    mirror.translate(2 * c.x(), 0);
    mirror.scale(-1, 1);
    const QPainterPath right = mirror.map(left);
    p.setPen(Qt::NoPen);
    p.setBrush(gold);
    p.drawPath(left);
    p.drawPath(right);

    // Title
    QFont titleFont = font();
    titleFont.setPointSize(28);
    titleFont.setBold(true);
    p.setFont(titleFont);
    p.setPen(QColor("#ffffff"));
    p.drawText(QRect(0, 165, width(), 60), Qt::AlignCenter, "سامانه مدیریت کتابخانه گنج");

    // Gold divider
    p.setPen(QPen(gold, 2));
    p.drawLine(QPointF(width() / 2.0 - 70, 238), QPointF(width() / 2.0 + 70, 238));

    // Subtitle
    QFont subFont = font();
    subFont.setPointSize(14);
    p.setFont(subFont);
    p.setPen(QColor("#d6e5df"));
    p.drawText(QRect(0, 250, width(), 40), Qt::AlignCenter, "تیپ ۱۱۶ پیاده مکانیزه شهید سرلشگر صفوی");

    // Progress bar
    const QRectF track(110, 330, width() - 220, 8);
    p.setPen(Qt::NoPen);
    p.setBrush(QColor(255, 255, 255, 40));
    p.drawRoundedRect(track, 4, 4);
    QRectF fill = track;
    fill.setWidth(track.width() * percent_ / 100.0);
    p.setBrush(gold);
    p.drawRoundedRect(fill, 4, 4);

    // Status message
    QFont msgFont = font();
    msgFont.setPointSize(10);
    p.setFont(msgFont);
    p.setPen(QColor("#9dbbb0"));
    p.drawText(QRect(0, 348, width(), 30), Qt::AlignCenter, message_);
}
