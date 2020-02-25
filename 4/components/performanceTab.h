#ifndef _PERFORMANCE_TAB_H
#define _PERFORMANCE_TAB_H

#include <QProgressBar>
#include <QLabel>
#include <QtCharts/QChartView>
#include <QtCharts/QLineSeries>

#include "../core/monitor.h"

QT_CHARTS_USE_NAMESPACE

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
    QVector<QPointF> cpuUsagePoints;
    QLineSeries *cpuUsageSeries;
    const int POINTS_NUMBER = 60;
};

#endif //_PERFORMANCE_TAB_H
