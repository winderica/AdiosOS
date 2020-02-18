#ifndef _SYSTEM_TAB_H
#define _SYSTEM_TAB_H

#include "../core/monitor.h"

class SystemTab : public QWidget {
Q_OBJECT

public:
    explicit SystemTab(Monitor &monitor, QWidget *parent = nullptr);

    ~SystemTab() override;
};

#endif //_SYSTEM_TAB_H
