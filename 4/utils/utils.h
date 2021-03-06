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
    using ull = unsigned long long;

    static string formatTime(chrono::duration<double> time);

    static string formatTimePoint(chrono::time_point<chrono::system_clock> timePoint);

    // convert size into human-friendly format
    static string formatSize(double originalSize);

    static string formatPercentage(double d);

    static vector<string> split(const string &str, const string &delimiter);

    static ull parseLine(const string &data);

};

#endif // _UTILS_H
