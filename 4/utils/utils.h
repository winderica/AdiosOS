#ifndef _UTILS_H
#define _UTILS_H

#include <iostream>
#include <chrono>
#include <iomanip>

#define KB              1000
#define MB              1000000
#define GB              1000000000
#define TB              1000000000000

using namespace std;

struct Utils {

    static stringstream formatTime(chrono::duration<double> time);

    static _Put_time<char> formatTimePoint(chrono::time_point<chrono::system_clock> timePoint);

    // convert size into human-friendly format
    static stringstream formatSize(double originalSize);

    static stringstream formatPercentage(double d);
};

#endif // _UTILS_H
