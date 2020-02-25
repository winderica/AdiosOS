#include <QVBoxLayout>
#include <QGroupBox>
#include <QFormLayout>
#include <QtCharts/QValueAxis>

#include "performanceTab.h"
#include "../utils/utils.h"

PerformanceTab::PerformanceTab(QWidget *parent) : QWidget(parent) {
    auto mainLayout = new QVBoxLayout();

    cpuUsagePoints = QVector<QPointF>(POINTS_NUMBER + 1, QPointF(0, 0));

    bootTimeLabel = new QLabel();
    idleTimeLabel = new QLabel();
    upTimeLabel = new QLabel();
    cpuUsageWidget = new QProgressBar();
    virtualMemoryUsageWidget = new QProgressBar();
    physicalMemoryUsageWidget = new QProgressBar();

    auto cpuTimeGroupBox = new QGroupBox("CPU Time");
    auto cpuTimeLayout = new QFormLayout();
    cpuTimeGroupBox->setLayout(cpuTimeLayout);

    cpuTimeLayout->addRow(new QLabel("bootTime:"), bootTimeLabel);
    cpuTimeLayout->addRow(new QLabel("idleTime:"), idleTimeLabel);
    cpuTimeLayout->addRow(new QLabel("upTime:"), upTimeLabel);

    auto cpuUsageChart = new QChart();
    cpuUsageSeries = new QLineSeries();
    cpuUsageChart->addSeries(cpuUsageSeries);
    cpuUsageChart->legend()->hide();
    auto axisX = new QValueAxis();
    axisX->setRange(-POINTS_NUMBER, 0);
    axisX->setLabelFormat("%g");
    axisX->setTitleText("Time/s");
    auto axisY = new QValueAxis();
    axisY->setRange(0, 100);
    axisY->setTitleText("Utilization/%");
    cpuUsageChart->addAxis(axisX, Qt::AlignBottom);
    cpuUsageChart->addAxis(axisY, Qt::AlignLeft);
    cpuUsageSeries->attachAxis(axisX);
    cpuUsageSeries->attachAxis(axisY);
    auto chartView = new QChartView(cpuUsageChart);
    chartView->setMinimumSize(800, 600);
    chartView->setRenderHint(QPainter::Antialiasing);

    auto cpuUsageGroupBox = new QGroupBox("CPU Usage");
    auto cpuUsageLayout = new QFormLayout();
    cpuUsageGroupBox->setLayout(cpuUsageLayout);

    cpuUsageLayout->addRow(chartView);
    cpuUsageLayout->addRow(cpuUsageWidget);

    auto memoryUsageGroupBox = new QGroupBox("Memory Usage");
    auto memoryUsageLayout = new QFormLayout();
    memoryUsageGroupBox->setLayout(memoryUsageLayout);

    memoryUsageLayout->addRow(virtualMemoryUsageWidget);
    memoryUsageLayout->addRow(physicalMemoryUsageWidget);

    mainLayout->addWidget(cpuTimeGroupBox);
    mainLayout->addWidget(cpuUsageGroupBox);
    mainLayout->addWidget(memoryUsageGroupBox);
    mainLayout->addStretch(1);
    setLayout(mainLayout);
}

PerformanceTab::~PerformanceTab() {
    delete cpuUsageWidget;
    delete virtualMemoryUsageWidget;
    delete physicalMemoryUsageWidget;
    delete bootTimeLabel;
    delete idleTimeLabel;
    delete upTimeLabel;
    delete cpuUsageSeries;
}

void PerformanceTab::updateData(Monitor::TimeInfo cpuTime, double cpuUsage, Monitor::MemoryInfo memoryInfo) {
    bootTimeLabel->setText(Utils::formatTimePoint(cpuTime.bootTime).c_str());
    idleTimeLabel->setText(Utils::formatTime(cpuTime.idleTime).c_str());
    upTimeLabel->setText(Utils::formatTime(cpuTime.upTime).c_str());

    cpuUsagePoints.pop_front();
    cpuUsagePoints.push_back(QPointF(0, cpuUsage * 100));
    int i = -POINTS_NUMBER;
    for (auto &data: cpuUsagePoints) {
        data.setX(i++);
    }
    cpuUsageSeries->replace(cpuUsagePoints);

    cpuUsageWidget->setFormat(QString("CPU: %1%").arg(cpuUsage * 100, 0, 'g', 3));
    cpuUsageWidget->setValue(cpuUsage * 100);
    cpuUsageWidget->setPalette(getColorPalette(cpuUsage));

    auto[totalVirtual, usedVirtual, totalPhysical, usedPhysical] = memoryInfo;
    auto virtualMemoryUsage = (double) usedVirtual / totalVirtual;
    auto physicalMemoryUsage = (double) usedPhysical / totalPhysical;
    virtualMemoryUsageWidget->setFormat(
        QString("Virtual Memory: %1% %2/%3")
            .arg(virtualMemoryUsage * 100, 0, 'g', 3)
            .arg(QString::fromStdString(Utils::formatSize(usedVirtual)))
            .arg(QString::fromStdString(Utils::formatSize(totalVirtual)))
    );
    virtualMemoryUsageWidget->setValue(virtualMemoryUsage * 100);
    virtualMemoryUsageWidget->setPalette(getColorPalette(virtualMemoryUsage));

    physicalMemoryUsageWidget->setFormat(
        QString("Physical Memory: %1% %2/%3")
            .arg(physicalMemoryUsage * 100, 0, 'g', 3)
            .arg(QString::fromStdString(Utils::formatSize(usedPhysical)))
            .arg(QString::fromStdString(Utils::formatSize(totalPhysical)))
    );
    physicalMemoryUsageWidget->setValue(physicalMemoryUsage * 100);
    physicalMemoryUsageWidget->setPalette(getColorPalette(physicalMemoryUsage));
}

QPalette PerformanceTab::getColorPalette(double percentage) {
    auto p = palette();
    p.setColor(QPalette::Highlight, QColor::fromRgb(
        0x8b + percentage * (0xf4 - 0x8b),
        0xc3 + percentage * (0x43 - 0xc3),
        0x4a + percentage * (0x36 - 0x4a)
    ));
    return p;
}
