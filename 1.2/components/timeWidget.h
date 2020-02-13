#ifndef _TIME_WIDGET_H
#define _TIME_WIDGET_H

#include <QtWidgets/QTextBrowser>

class TimeWidget : public QTextBrowser {
Q_OBJECT

public:
    explicit TimeWidget(QWidget *parent = nullptr);

    ~TimeWidget() override = default;

private slots:

    void showTime();
};

#endif // _TIME_WIDGET_H
