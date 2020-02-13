#include <QTimer>
#include "counterWidget.h"

CounterWidget::CounterWidget(QWidget *parent) : QTextBrowser(parent) {
    auto timer = new QTimer(this);
    connect(timer, &QTimer::timeout, this, &CounterWidget::showCount);
    timer->start(1000);
}

void CounterWidget::showCount() {
    setText(QString::number(count));
    count = (count + 1) % 10;
}
