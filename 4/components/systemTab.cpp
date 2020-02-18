#include <QVBoxLayout>
#include <QGroupBox>
#include <QFormLayout>
#include <QLabel>

#include "systemTab.h"

SystemTab::SystemTab(Monitor &monitor, QWidget *parent) : QWidget(parent) {
    auto mainLayout = new QVBoxLayout();

    auto osGroupBox = new QGroupBox("OS");
    auto osLayout = new QFormLayout();
    osGroupBox->setLayout(osLayout);

    osLayout->addRow(new QLabel("Hostname:"), new QLabel(QString::fromStdString(monitor.getHostname())));
    osLayout->addRow(new QLabel("Kernel:"), new QLabel(QString::fromStdString(monitor.getVersion())));

    auto cpuInfo = monitor.getCPUInfo();
    auto cpuGroupBox = new QGroupBox("CPU");
    auto cpuLayout = new QFormLayout();
    cpuGroupBox->setLayout(cpuLayout);

    cpuLayout->addRow(new QLabel("Vendor:"), new QLabel(QString::fromStdString(cpuInfo.vendorId)));
    cpuLayout->addRow(new QLabel("Model:"), new QLabel(QString::fromStdString(cpuInfo.modelName)));
    cpuLayout->addRow(new QLabel("Processors:"), new QLabel(QString::number(cpuInfo.processors)));
    cpuLayout->addRow(new QLabel("Cores:"), new QLabel(QString::number(cpuInfo.cores)));
    cpuLayout->addRow(new QLabel("Frequency:"), new QLabel(QString::fromStdString(cpuInfo.frequency + " MHz")));
    cpuLayout->addRow(new QLabel("Cache:"), new QLabel(QString::fromStdString(cpuInfo.cache)));

    mainLayout->addWidget(osGroupBox);
    mainLayout->addWidget(cpuGroupBox);
    mainLayout->addStretch(1);
    setLayout(mainLayout);
}

SystemTab::~SystemTab() = default;
