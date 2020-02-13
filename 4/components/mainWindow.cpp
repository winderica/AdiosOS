#include <QGridLayout>
#include <QHeaderView>
#include <QDebug>
#include <QtWidgets/QDesktopWidget>

#include "mainWindow.h"
#include "../utils/utils.h"

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent) {
    auto widget = new QWidget();
    auto mainLayout = new QGridLayout();

    hostnameWidget = new QLabel(QString::fromStdString("Hostname: " + monitor.getHostname()));
    versionWidget = new QLabel(QString::fromStdString("Kernel: " + monitor.getVersion()));
    auto cpuInfo = monitor.getCPUInfo();
    cpuInfoWidget = new QLabel(QString::fromStdString(
        "Vendor: " + cpuInfo.vendorId
        + "\nModel: " + cpuInfo.modelName
        + "\nProcessors: " + to_string(cpuInfo.processors)
        + "\nCores: " + to_string(cpuInfo.cores)
        + "\nFrequency: " + cpuInfo.frequency
        + "\nCache: " + cpuInfo.cache
    ));
    cpuTimeWidget = new QLabel();

    cpuUsageWidget = new QProgressBar();
    virtualMemoryUsageWidget = new QProgressBar();
    physicalMemoryUsageWidget = new QProgressBar();

    processList = new QTableWidget();
    processList->setColumnCount(15);
    QStringList processListHeader;
    processListHeader << "PID" << "PPID" << "UID" << "USER" << "NAME"
                      << "PRI" << "NI" << "VIRT" << "RES" << "SHR"
                      << "S" << "CPU%" << "MEM%" << "TIME+" << "COMMAND";
    processList->setHorizontalHeaderLabels(processListHeader);
    processList->horizontalHeader()->setStretchLastSection(true);
    QList<int> widths = {
        50, 50, 50, 100, 100,
        25, 25, 50, 50, 50,
        20, 50, 50, 120 // Omit the last column
    };
    for (int i = 0; i < widths.size(); i++) {
        processList->setColumnWidth(i, widths[i]);
    }
    processList->setEditTriggers(QAbstractItemView::NoEditTriggers);


    mainLayout->addWidget(hostnameWidget, 0, 0, 1, 3);
    mainLayout->addWidget(versionWidget, 0, 3, 1, 3);
    mainLayout->addWidget(cpuInfoWidget, 1, 0, 1, 3);
    mainLayout->addWidget(cpuTimeWidget, 1, 3, 1, 3);
    mainLayout->addWidget(cpuUsageWidget, 2, 0, 1, 6);
    mainLayout->addWidget(virtualMemoryUsageWidget, 3, 0, 1, 6);
    mainLayout->addWidget(physicalMemoryUsageWidget, 4, 0, 1, 6);
    mainLayout->addWidget(processList, 5, 0, 1, 6);

    widget->setLayout(mainLayout);
    setCentralWidget(widget);
    resize(QDesktopWidget().availableGeometry(this).size());
    startTimer(1000);
}

QStringList &operator<<(QStringList &sl, double d) {
    sl << QString::number(d);
    return sl;
}

QStringList &operator<<(QStringList &sl, unsigned long long d) {
    sl << QString::number(d);
    return sl;
}

QStringList &operator<<(QStringList &sl, string &d) {
    sl << QString::fromStdString(d);
    return sl;
}

QStringList &operator<<(QStringList &sl, stringstream d) {
    sl << QString::fromStdString(d.str());
    return sl;
}

QPalette MainWindow::getColorPalette(double percentage) {
    auto p = palette();
    p.setColor(QPalette::Highlight, QColor::fromRgb(
        0x8b + percentage * (0xf4 - 0x8b),
        0xc3 + percentage * (0x43 - 0xc3),
        0x4a + percentage * (0x36 - 0x4a)
    ));
    return p;
}

void MainWindow::timerEvent(QTimerEvent *) {
    auto cpuTime = monitor.getCPUTime();
    stringstream ss;
    ss << "bootTime: " << Utils::formatTimePoint(cpuTime.bootTime) << endl
       << "idleTime: " << Utils::formatTime(cpuTime.idleTime).rdbuf() << endl
       << "upTime: " << Utils::formatTime(cpuTime.upTime).rdbuf();
    cpuTimeWidget->setText(QString::fromStdString(ss.str()));

    auto usage = monitor.getUsage();
    auto cpuUsage = usage.cpuUsage;
    cpuUsageWidget->setFormat(QString("CPU: %1%").arg(cpuUsage * 100, 0, 'g', 3));
    cpuUsageWidget->setValue(cpuUsage * 100);
    cpuUsageWidget->setPalette(getColorPalette(cpuUsage));

    auto[totalVirtual, usedVirtual, totalPhysical, usedPhysical] = monitor.getMemoryInfo();
    auto virtualMemoryUsage = (double) usedVirtual / totalVirtual;
    auto physicalMemoryUsage = (double) usedPhysical / totalPhysical;
    virtualMemoryUsageWidget->setFormat(
        QString("Virtual Memory: %1% %2/%3")
            .arg(virtualMemoryUsage * 100, 0, 'g', 3)
            .arg(QString::fromStdString(Utils::formatSize(usedVirtual).str()))
            .arg(QString::fromStdString(Utils::formatSize(totalVirtual).str()))
    );
    virtualMemoryUsageWidget->setValue(virtualMemoryUsage * 100);
    virtualMemoryUsageWidget->setPalette(getColorPalette(virtualMemoryUsage));

    physicalMemoryUsageWidget->setFormat(
        QString("Physical Memory: %1% %2/%3")
            .arg(physicalMemoryUsage * 100, 0, 'g', 3)
            .arg(QString::fromStdString(Utils::formatSize(usedPhysical).str()))
            .arg(QString::fromStdString(Utils::formatSize(totalPhysical).str()))
    );
    physicalMemoryUsageWidget->setValue(physicalMemoryUsage * 100);
    physicalMemoryUsageWidget->setPalette(getColorPalette(physicalMemoryUsage));

    auto processes = usage.processes;
    int rowNum = 0;
    for (auto[_, process]: processes) {
        auto[
        pid,
        uid,
        parentPid,
        virtualMemory,
        physicalMemory,
        sharedMemory,
        priority,
        nice,
        memoryUsage,
        cpuUsage,
        time,
        name,
        user,
        command,
        state
        ] = process;
        QStringList row;
        row << pid << parentPid << uid << user << name << priority << nice
            << Utils::formatSize(virtualMemory)
            << Utils::formatSize(physicalMemory)
            << Utils::formatSize(sharedMemory)
            << state
            << Utils::formatPercentage(cpuUsage)
            << Utils::formatPercentage(memoryUsage)
            << Utils::formatTime(time) // CLion infers `time` as `int`, which is actually `duration<double>`
            << command;
        if (processList->rowCount() == rowNum) {
            processList->insertRow(rowNum);
        }
        int columnNum = 0;
        for (const auto &data: row) {
            auto item = new QTableWidgetItem(data);
            processList->setItem(rowNum, columnNum++, item);
        }
        rowNum++;
    }
    while (rowNum < processList->rowCount()) {
        processList->removeRow(processList->rowCount() - 1);
    }
}

MainWindow::~MainWindow() {
    delete cpuUsageWidget;
    delete virtualMemoryUsageWidget;
    delete physicalMemoryUsageWidget;
    delete processList;
    delete hostnameWidget;
    delete versionWidget;
    delete cpuInfoWidget;
}
