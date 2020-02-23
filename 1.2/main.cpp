#include <QApplication>
#include <sys/wait.h>
#include <unistd.h>

#include "components/timeWidget.h"
#include "components/counterWidget.h"
#include "components/sumWidget.h"

int main(int argc, char *argv[]) {
    auto p1 = fork();
    if (p1 == 0) { // p1
        QApplication app(argc, argv);
        TimeWidget timeWidget;
        timeWidget.show();
        app.exec();
    } else {
        auto p2 = fork();
        if (p2 == 0) { // p2
            QApplication app(argc, argv);
            CounterWidget counterWidget;
            counterWidget.show();
            app.exec();
        } else {
            auto p3 = fork();
            if (p3 == 0) { // p3
                QApplication app(argc, argv);
                SumWidget sumWidget;
                sumWidget.show();
                app.exec();
            } else { // main
                waitpid(p1, nullptr, 0);
                waitpid(p2, nullptr, 0);
                waitpid(p3, nullptr, 0);
                return 0;
            }
        }
    }
}