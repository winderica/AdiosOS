#include "mainWindow.h"

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent) {
    auto tabWidget = new QTabWidget();
    performanceTab = new PerformanceTab();
    processTab = new ProcessTab();
    auto systemTab = new SystemTab(monitor);
    auto aboutTab = new AboutTab();
    auto moduleTab = new ModuleTab(monitor);

    tabWidget->addTab(performanceTab, "Performance");
    tabWidget->addTab(processTab, "Processes");
    tabWidget->addTab(moduleTab, "Modules");
    tabWidget->addTab(systemTab, "System");
    tabWidget->addTab(aboutTab, "About");

    setCentralWidget(tabWidget);
    resize(1280, 720);
    startTimer(1000);
}

void MainWindow::timerEvent(QTimerEvent *) {
    auto usage = monitor.getUsage();
    performanceTab->updateData(monitor.getCPUTime(), usage.cpuUsage, monitor.getMemoryInfo());
    processTab->updateData(usage.processes);
}

MainWindow::~MainWindow() {
    delete performanceTab;
    delete processTab;
}
