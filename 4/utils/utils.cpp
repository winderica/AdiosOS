#include "utils.h"

string Utils::formatTime(chrono::duration<double> time) {
    using namespace chrono;
    stringstream ss;
    char fill = ss.fill();
    ss.fill('0');
    auto d = duration_cast<duration<int, ratio<86400>>>(time);
    time -= d;
    auto h = duration_cast<hours>(time);
    time -= h;
    auto m = duration_cast<minutes>(time);
    time -= m;
    auto s = duration_cast<seconds>(time);
    time -= s;
    auto c = duration_cast<duration<int, centi>>(time);
    ss << setw(2) << d.count() << "d:"
       << setw(2) << h.count() << "h:"
       << setw(2) << m.count() << "m:"
       << setw(2) << s.count() << "."
       << setw(2) << c.count() << "s";
    ss.fill(fill);
    return ss.str();
}

string Utils::formatTimePoint(chrono::time_point<chrono::system_clock> timePoint) {
    stringstream ss;
    auto t = chrono::system_clock::to_time_t(timePoint);
    ss << put_time(localtime(&t), "%F %T");
    return ss.str();
}

string Utils::formatSize(double originalSize) {
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
    return ss.str();
}

string Utils::formatPercentage(double d) {
    stringstream ss;
    ss << fixed << setprecision(1) << d * 100 << "%";
    return ss.str();
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

Utils::ull Utils::parseLine(const string &data) {
    ull x = 0;
    auto i = 0;
    // fast read, not safe for general usage
    while (data[i] < '0' || data[i] > '9') {
        i++;
    }
    while (data[i] >= '0' && data[i] <= '9') {
        x = (x << 3) + (x << 1) + (data[i] - '0');
        i++;
    }
    return x;
}
