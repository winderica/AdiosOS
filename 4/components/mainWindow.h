#ifndef _MAIN_WINDOW_H
#define _MAIN_WINDOW_H

#include <QMainWindow>

#include "../core/monitor.h"
#include "performanceTab.h"
#include "systemTab.h"
#include "processTab.h"
#include "aboutTab.h"
#include "moduleTab.h"

class MainWindow : public QMainWindow {
Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);

    ~MainWindow() override;

protected:
    void timerEvent(QTimerEvent *) override;

private:
    PerformanceTab *performanceTab;
    ProcessTab *processTab;
    Monitor monitor;
};

#endif // _MAIN_WINDOW_H
