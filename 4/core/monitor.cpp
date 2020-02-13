#include <fstream>
#include <regex>
#include <filesystem>
#include <unistd.h>
#include <pwd.h>
#include <iostream>

#include "monitor.h"

vector<string> Monitor::split(const string &str, const string &delimiter) {
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

Monitor::ull Monitor::parseLine(const string &data) {
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

void Monitor::readJiffies() {
    ifstream cpuUsage("/proc/stat");
    string line;
    getline(cpuUsage, line);
    auto values = split(line, " ");
    usage.currentJiffies.user = stoull(values[1]);
    usage.currentJiffies.nice = stoull(values[2]);
    usage.currentJiffies.system = stoull(values[3]);
    usage.currentJiffies.idle = stoull(values[4]);
    usage.currentJiffies.iowait = stoull(values[5]);
    usage.currentJiffies.irq = stoull(values[6]);
    usage.currentJiffies.softirq = stoull(values[7]);
}

pair<double, double> Monitor::readUpTime() {
    ifstream uptime("/proc/uptime");
    string line;
    getline(uptime, line);
    auto values = split(line, " ");
    return {stod(values[0]), stod(values[1])};
}

string Monitor::getVersion() {
    return version;
}

void Monitor::readVersion() {
    ifstream ifs("/proc/version");
    version = string(is_iter(ifs), is_iter());
}

Monitor::CPUInfo Monitor::getCPUInfo() {
    return cpuInfo;
}

void Monitor::readCPUInfo() {
    ifstream ifs("/proc/cpuinfo");
    string line;
    // Use regexps here because cpu info is only fetched once
    regex re_vendor(R"(vendor_id\s*:\s*(.*))");
    regex re_model(R"(model name\s*:\s*(.*))");
    regex re_frequency(R"(cpu MHz\s*:\s*(.*))");
    regex re_cache(R"(cache size\s*:\s*(.*))");
    regex re_siblings(R"(siblings\s*:\s*(.*))");
    regex re_cores(R"(cpu cores\s*:\s*(.*))");
    while (getline(ifs, line)) {
        smatch m;
        if (line.empty()) {
            break;
        } else if (regex_search(line, m, re_vendor)) {
            cpuInfo.vendorId = m[1];
        } else if (regex_search(line, m, re_model)) {
            cpuInfo.modelName = m[1];
        } else if (regex_search(line, m, re_frequency)) {
            cpuInfo.frequency = m[1];
        } else if (regex_search(line, m, re_cache)) {
            cpuInfo.cache = m[1];
        } else if (regex_search(line, m, re_siblings)) {
            cpuInfo.processors = stoi(m[1]);
        } else if (regex_search(line, m, re_cores)) {
            cpuInfo.cores = stoi(m[1]);
        }
    }
}

Monitor::MemoryInfo Monitor::getMemoryInfo() {
    struct sysinfo memInfo{};
    sysinfo(&memInfo);
    auto unit = memInfo.mem_unit;
    ull totalPhysical = memInfo.totalram;
    ull usedPhysical = totalPhysical - memInfo.freeram;
    ull totalVirtual = totalPhysical + memInfo.totalswap;
    ull usedVirtual = totalVirtual - memInfo.freeram - memInfo.freeswap;
    return {
        totalVirtual * unit,
        usedVirtual * unit,
        totalPhysical * unit,
        usedPhysical * unit
    };
}

Monitor::Usage Monitor::getUsage() {
    using namespace chrono;
    namespace fs = filesystem;
    auto prevBusy = usage.currentJiffies.busy();
    auto prevTotal = usage.currentJiffies.total();
    readJiffies();
    auto diffBusy = (double) (usage.currentJiffies.busy() - prevBusy);
    auto diffTotal = (double) (usage.currentJiffies.total() - prevTotal);
    auto cpuCount = cpuInfo.processors;
    usage.cpuUsage = diffBusy / diffTotal;
    vector<ull> killed;
    for (auto[pid, _]: usage.processes) {
        if (!fs::exists(fs::path("/proc") / to_string(pid))) {
            killed.push_back(pid);
        }
    }
    for (auto pid: killed) {
        usage.processes.erase(pid);
    }
    for (const auto &p: fs::directory_iterator("/proc")) {
        const auto &path = p.path();
        auto filename = path.filename().string();
        auto isProcess = true;
        for (auto ch: filename) {
            if (ch < '0' || ch > '9') {
                isProcess = false;
                break;
            }
        }
        if (p.is_directory() && isProcess) {
            ifstream status(path / "status");
            ifstream stat(path / "stat");
            ifstream command(path / "cmdline");
            string line;
            ProcessInfo process{};
            auto fields = split(string(is_iter(stat), is_iter()), " ");
            auto fieldsCount = 52;
            if (fields.size() != fieldsCount) {
                continue;
            }
            process.pid = stoull(fields[0]);
            process.state = fields[2];
            process.parentPid = stoull(fields[3]);
            auto hz = sysconf(_SC_CLK_TCK);
            auto uTime = stod(fields[13]);
            auto sTime = stod(fields[14]);
//            auto cuTime = stod(fields[15]);
//            auto csTime = stod(fields[16]);
            auto totalTime = (uTime + sTime) / hz;
            auto prevTime = usage.processes.count(process.pid) ? usage.processes[process.pid].time.count() : 0;
            process.time = duration<double>(totalTime);
            process.priority = stoull(fields[17]);
            process.nice = stoull(fields[18]);
            process.cpuUsage = (totalTime - prevTime) * hz * cpuCount / diffTotal;
            process.command = string(is_iter(command), is_iter());
            replace(process.command.begin(), process.command.end(), '\0', ' ');
            while (getline(status, line)) {
                if (line.substr(0, 4) == "Name") {
                    process.name = line.substr(6); // magic number 6 is the start of name value
                } else if (line.substr(0, 6) == "VmSize") {
                    process.virtualMemory = parseLine(line) * 1000;
                } else if (line.substr(0, 5) == "VmRSS") {
                    process.physicalMemory = parseLine(line) * 1000;
                } else if (line.substr(0, 3) == "Uid") {
                    process.uid = parseLine(line);
                } else if (line.substr(0, 7) == "RssFile") {
                    process.sharedMemory = parseLine(line) * 1000;
                } else if (line.substr(0, 8) == "RssShmem") {
                    process.sharedMemory += parseLine(line) * 1000;
                }
            }
            process.memoryUsage = (double) process.physicalMemory / getMemoryInfo().totalPhysical;
            if (!user.count(process.uid)) {
                auto pw = getpwuid(process.uid);
                user[process.uid] = string(pw->pw_name);
            }
            process.user = user[process.uid];
            usage.processes[process.pid] = process;
        }
    }
    return usage;
}

Monitor::TimeInfo Monitor::getCPUTime() {
    using namespace chrono;
    auto upTime = duration<double>(readUpTime().first);
    auto idleTime = duration<double>(readUpTime().second / cpuInfo.processors);
    auto now = system_clock::now();
    auto bootTime = now - duration_cast<seconds>(upTime);
    return {
        upTime,
        idleTime,
        bootTime
    };
}

void Monitor::readHostname() {
    ifstream ifs("/proc/sys/kernel/hostname");
    hostname = string(is_iter(ifs), is_iter());
}

string Monitor::getHostname() {
    return hostname;
}

Monitor::Monitor() {
    readCPUInfo();
    readVersion();
    readHostname();
}

Monitor::~Monitor() = default;
