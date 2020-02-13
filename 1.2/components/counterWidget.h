#ifndef _COUNTER_WIDGET_H
#define _COUNTER_WIDGET_H

#include <QtWidgets/QTextBrowser>

class CounterWidget : public QTextBrowser {
Q_OBJECT

public:
    explicit CounterWidget(QWidget *parent = nullptr);

    ~CounterWidget() override = default;

private slots:

    void showCount();

private:
    int count = 0;
};

#endif // _COUNTER_WIDGET_H
