#include <QHeaderView>
#include <QVBoxLayout>
#include <QLabel>

#include "../utils/utils.h"
#include "moduleTab.h"

ModuleTab::ModuleTab(Monitor &monitor, QWidget *parent) : QWidget(parent) {
    auto mainLayout = new QVBoxLayout();

    auto moduleTable = new QTableWidget();
    moduleTable->setColumnCount(5);
    QStringList processListHeader;
    processListHeader << "Name" << "MemorySize" << "Instances" << "State" << "Dependencies";
    moduleTable->setHorizontalHeaderLabels(processListHeader);
    moduleTable->horizontalHeader()->setStretchLastSection(true);
    QList<int> widths = {
        200, 100, 100, 50 // Omit the last column
    };
    for (int i = 0; i < widths.size(); i++) {
        moduleTable->setColumnWidth(i, widths[i]);
    }
    moduleTable->setEditTriggers(QAbstractItemView::NoEditTriggers);

    auto modules = monitor.getModules();
    int rowNum = 0;
    for (const auto &module: modules) {
        auto[name, memorySize, instanceCount, dependencies, state] = module;
        QStringList row;
        row << name.c_str() << Utils::formatSize(memorySize).c_str()
            << QString::number(instanceCount) << state.c_str() << dependencies.c_str();
        moduleTable->insertRow(rowNum);
        int columnNum = 0;
        for (const auto &data: row) {
            auto item = new QTableWidgetItem(data);
            moduleTable->setItem(rowNum, columnNum++, item);
        }
        rowNum++;
    }

    mainLayout->addWidget(moduleTable);
    setLayout(mainLayout);
}

ModuleTab::~ModuleTab() = default;
