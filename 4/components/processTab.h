#ifndef _PROCESS_TAB_H
#define _PROCESS_TAB_H

#include <QTableWidget>

#include "../core/monitor.h"

class ProcessTab : public QWidget {
Q_OBJECT

public:
    explicit ProcessTab(QWidget *parent = nullptr);

    ~ProcessTab() override;

    void updateData(const map<Monitor::ull, Monitor::ProcessInfo> &processes);

private:
    QTableWidget *processTable;

};


#endif // _PROCESS_TAB_H
