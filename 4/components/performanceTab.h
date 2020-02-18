#ifndef _PERFORMANCE_TAB_H
#define _PERFORMANCE_TAB_H

#include <QProgressBar>
#include <QLabel>

#include "../core/monitor.h"

class PerformanceTab : public QWidget {
Q_OBJECT

public:
    explicit PerformanceTab(QWidget *parent = nullptr);

    ~PerformanceTab() override;

    void updateData(Monitor::TimeInfo cpuTime, double cpuUsage, Monitor::MemoryInfo memoryInfo);

private:
    QPalette getColorPalette(double percentage);

    QProgressBar *cpuUsageWidget;
    QProgressBar *virtualMemoryUsageWidget;
    QProgressBar *physicalMemoryUsageWidget;
    QLabel *bootTimeLabel;
    QLabel *idleTimeLabel;
    QLabel *upTimeLabel;
};

#endif //_PERFORMANCE_TAB_H
