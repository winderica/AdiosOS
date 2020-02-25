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

vector<string> Utils::split(const string &str, const string &delimiter) {
    vector<string> tokens;
    size_t prev = 0, pos;
    do {
        pos = str.find(delimiter, prev);
        if (pos == string::npos) {
            pos = str.length();
        }
        auto token = str.substr(prev, pos - prev);
        if (!token.empty()) {
            tokens.push_back(token);
        }
        prev = pos + delimiter.length();
    } while (pos < str.length() && prev < str.length());
    return tokens;
}