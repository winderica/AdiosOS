#include <QVBoxLayout>
#include <QGroupBox>
#include <QFormLayout>

#include "performanceTab.h"
#include "../utils/utils.h"

PerformanceTab::PerformanceTab(QWidget *parent) : QWidget(parent) {
    auto mainLayout = new QVBoxLayout();

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

    auto cpuUsageGroupBox = new QGroupBox("CPU Usage");
    auto cpuUsageLayout = new QFormLayout();
    cpuUsageGroupBox->setLayout(cpuUsageLayout);

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
}

void PerformanceTab::updateData(Monitor::TimeInfo cpuTime, double cpuUsage, Monitor::MemoryInfo memoryInfo) {
    bootTimeLabel->setText(Utils::formatTimePoint(cpuTime.bootTime).c_str());
    idleTimeLabel->setText(Utils::formatTime(cpuTime.idleTime).c_str());
    upTimeLabel->setText(Utils::formatTime(cpuTime.upTime).c_str());

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
