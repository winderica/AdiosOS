#include <QTimer>
#include "sumWidget.h"

SumWidget::SumWidget(QWidget *parent) : QTextBrowser(parent) {
    auto timer = new QTimer(this);
    connect(timer, &QTimer::timeout, this, &SumWidget::showSum);
    timer->start(1000);
}

void SumWidget::showSum() {
    setText(QString::number(sum));
    if (next <= 1000) {
        sum += next;
        next++;
    }
}
