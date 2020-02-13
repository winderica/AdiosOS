#ifndef _SUM_WIDGET_H
#define _SUM_WIDGET_H

#include <QtWidgets/QTextBrowser>

class SumWidget : public QTextBrowser {
Q_OBJECT

public:
    explicit SumWidget(QWidget *parent = nullptr);

    ~SumWidget() override = default;

private slots:

    void showSum();

private:
    int sum = 0;
    int next = 1;
};

#endif // _SUM_WIDGET_H
