#ifndef _UTILS_H
#define _UTILS_H

#include <iostream>
#include <chrono>
#include <iomanip>
#include <vector>

#define KB              1000
#define MB              1000000
#define GB              1000000000
#define TB              1000000000000

using namespace std;

struct Utils {
    static vector<string> split(const string &str, const string &delimiter);

    static _Put_time<char> formatTimePoint(uint32_t timePoint);

    static stringstream formatSize(double originalSize);
};

#endif // _UTILS_H
