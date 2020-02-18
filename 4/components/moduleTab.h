#ifndef _MODULE_TAB_H
#define _MODULE_TAB_H

#include <QTableWidget>

#include "../core/monitor.h"

class ModuleTab : public QWidget {
Q_OBJECT

public:
    explicit ModuleTab(Monitor &monitor, QWidget *parent = nullptr);

    ~ModuleTab() override;
};

#endif // _MODULE_TAB_H
