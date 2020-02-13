#ifndef _MAIN_WINDOW_H
#define _MAIN_WINDOW_H

#include <QMainWindow>
#include <QProgressBar>
#include <QTableWidget>
#include <QLabel>
#include "../core/monitor.h"

using namespace std;

class MainWindow : public QMainWindow {
Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);

    ~MainWindow() override;

protected:
    void timerEvent(QTimerEvent *) override;

private:
    QPalette getColorPalette(double percentage);

    QProgressBar *cpuUsageWidget;
    QProgressBar *virtualMemoryUsageWidget;
    QProgressBar *physicalMemoryUsageWidget;
    QTableWidget *processList;
    QLabel *hostnameWidget;
    QLabel *versionWidget;
    QLabel *cpuInfoWidget;
    QLabel *cpuTimeWidget;
    Monitor monitor;
};

#endif // _MAIN_WINDOW_H
