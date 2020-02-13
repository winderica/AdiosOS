#include <QGridLayout>
#include <QTimer>
#include <QTime>
#include "timeWidget.h"

TimeWidget::TimeWidget(QWidget *parent) : QTextBrowser(parent) {
    auto timer = new QTimer(this);
    connect(timer, &QTimer::timeout, this, &TimeWidget::showTime);
    timer->start(1000);
}

void TimeWidget::showTime() {
    auto time = QTime::currentTime();
    auto text = time.toString("hh:mm:ss");
    setText(text);
}
