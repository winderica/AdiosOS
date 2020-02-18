#include <QHeaderView>
#include <QVBoxLayout>
#include <QLabel>

#include "../utils/utils.h"
#include "processTab.h"

ProcessTab::ProcessTab(QWidget *parent) : QWidget(parent) {
    auto mainLayout = new QVBoxLayout();

    processTable = new QTableWidget();
    processTable->setColumnCount(15);
    QStringList processListHeader;
    processListHeader << "PID" << "PPID" << "UID" << "USER" << "NAME"
                      << "PRI" << "NI" << "VIRT" << "RES" << "SHR"
                      << "S" << "CPU%" << "MEM%" << "TIME+" << "COMMAND";
    processTable->setHorizontalHeaderLabels(processListHeader);
    processTable->horizontalHeader()->setStretchLastSection(true);
    QList<int> widths = {
        50, 50, 50, 100, 100,
        25, 25, 50, 50, 50,
        20, 50, 50, 120 // Omit the last column
    };
    for (int i = 0; i < widths.size(); i++) {
        processTable->setColumnWidth(i, widths[i]);
    }
    processTable->setEditTriggers(QAbstractItemView::NoEditTriggers);

    mainLayout->addWidget(processTable);
    setLayout(mainLayout);
}

ProcessTab::~ProcessTab() {
    delete processTable;
}

void ProcessTab::updateData(const map<Monitor::ull, Monitor::ProcessInfo> &processes) {
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
        row << QString::number(pid) << QString::number(parentPid) << QString::number(uid)
            << user.c_str() << name.c_str() << QString::number(priority) << QString::number(nice)
            << Utils::formatSize(virtualMemory).c_str() << Utils::formatSize(physicalMemory).c_str()
            << Utils::formatSize(sharedMemory).c_str() << state.c_str()
            << Utils::formatPercentage(cpuUsage).c_str() << Utils::formatPercentage(memoryUsage).c_str()
            << Utils::formatTime(time).c_str() << command.c_str();
        if (processTable->rowCount() == rowNum) {
            processTable->insertRow(rowNum);
        }
        int columnNum = 0;
        for (const auto &data: row) {
            auto item = new QTableWidgetItem(data);
            processTable->setItem(rowNum, columnNum++, item);
        }
        rowNum++;
    }
    while (rowNum < processTable->rowCount()) {
        processTable->removeRow(processTable->rowCount() - 1);
    }
}