#ifndef _ABOUT_TAB_H
#define _ABOUT_TAB_H

#include <QWidget>

class AboutTab : public QWidget {
Q_OBJECT

public:
    explicit AboutTab(QWidget *parent = nullptr);

    ~AboutTab() override;
};


#endif // _ABOUT_TAB_H
