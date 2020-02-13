#include "utils.h"

_Put_time<char> Utils::formatTimePoint(uint32_t timePoint) {
    time_t t = timePoint;
    return put_time(localtime(&t), "%F %T");
}

stringstream Utils::formatSize(double originalSize) {
    stringstream ss;
    double size = originalSize;
    ss << fixed;
    if (size < KB) {
        ss << setprecision(0) << size << "B";
    } else if (size < MB) {
        size /= KB;
        ss << setprecision(size < 10) << size << "K";
    } else if (size < GB) {
        size /= MB;
        ss << setprecision(size < 10) << size << "M";
    } else if (size < TB) {
        size /= GB;
        ss << setprecision(size < 10) << size << "G";
    } else {
        size /= TB;
        ss << setprecision(size < 10) << size << "T";
    }
    return ss;
}
