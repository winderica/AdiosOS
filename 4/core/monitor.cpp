#include <fstream>
#include <regex>
#include <filesystem>
#include <unistd.h>
#include <pwd.h>
#include <iostream>

#include "monitor.h"
#include "../utils/utils.h"

void Monitor::readJiffies() {
    ifstream cpuUsage("/proc/stat");
    string tmp, user, nice, system, idle, ioWait, irq, softIrq;
    cpuUsage >> tmp >> user >> nice >> system >> idle >> ioWait >> irq >> softIrq;
    usage.currentJiffies = {
        stoull(user),
        stoull(nice),
        stoull(system),
        stoull(idle),
        stoull(ioWait),
        stoull(irq),
        stoull(softIrq),
    };
}

string Monitor::getVersion() {
    return version;
}

void Monitor::readVersion() {
    ifstream ifs("/proc/version");
    getline(ifs, version);
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
    ifstream ifs("/proc/meminfo");
    string line;
    ull totalRAM;
    ull freeRAM;
    ull totalSwap;
    ull freeSwap;
    while (getline(ifs, line)) {
        if (line.substr(0, 8) == "MemTotal") {
            totalRAM = Utils::parseLine(line) * 1000;
        } else if (line.substr(0, 7) == "MemFree") {
            freeRAM = Utils::parseLine(line) * 1000;
        } else if (line.substr(0, 9) == "SwapTotal") {
            totalSwap = Utils::parseLine(line) * 1000;
        } else if (line.substr(0, 8) == "SwapFree") {
            freeSwap = Utils::parseLine(line) * 1000;
        }
    }
    ull totalPhysical = totalRAM;
    ull usedPhysical = totalPhysical - freeRAM;
    ull totalVirtual = totalPhysical + totalSwap;
    ull usedVirtual = totalVirtual - freeRAM - freeSwap;
    return {
        totalVirtual,
        usedVirtual,
        totalPhysical,
        usedPhysical
    };
}

Monitor::Usage Monitor::getUsage() {
    using is_iter = istreambuf_iterator<char>;
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
            auto fields = Utils::split(string(is_iter(stat), is_iter()), " ");
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
            auto currProcessTime = (uTime + sTime) / hz;
            auto prevProcessTime = usage.processes.count(process.pid) ? usage.processes[process.pid].time.count() : 0;
            process.time = duration<double>(currProcessTime);
            process.priority = stoull(fields[17]);
            process.nice = stoull(fields[18]);
            process.cpuUsage = (currProcessTime - prevProcessTime) * hz * cpuCount / diffTotal;
            process.command = string(is_iter(command), is_iter());
            replace(process.command.begin(), process.command.end(), '\0', ' ');
            while (getline(status, line)) {
                if (line.substr(0, 4) == "Name") {
                    process.name = line.substr(6); // magic number 6 is the start of name value
                } else if (line.substr(0, 6) == "VmSize") {
                    process.virtualMemory = Utils::parseLine(line) * 1000;
                } else if (line.substr(0, 5) == "VmRSS") {
                    process.physicalMemory = Utils::parseLine(line) * 1000;
                } else if (line.substr(0, 3) == "Uid") {
                    process.uid = Utils::parseLine(line);
                } else if (line.substr(0, 7) == "RssFile") {
                    process.sharedMemory = Utils::parseLine(line) * 1000;
                } else if (line.substr(0, 8) == "RssShmem") {
                    process.sharedMemory += Utils::parseLine(line) * 1000;
                }
            }
            process.memoryUsage = (double) process.physicalMemory / getMemoryInfo().totalPhysical;
            if (!users.count(process.uid)) {
                auto pw = getpwuid(process.uid);
                users[process.uid] = string(pw->pw_name);
            }
            process.user = users[process.uid];
            usage.processes[process.pid] = process;
        }
    }
    return usage;
}

Monitor::TimeInfo Monitor::getCPUTime() {
    using namespace chrono;
    ifstream ifs("/proc/uptime");
    string first, second;
    ifs >> first >> second;
    auto upTime = duration<double>(stod(first));
    auto idleTime = duration<double>(stod(second) / cpuInfo.processors);
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
    getline(ifs, hostname);
}

string Monitor::getHostname() {
    return hostname;
}

void Monitor::readModules() {
    ifstream ifs("/proc/modules");
//    stringstream ifs(R"(veth 20480 0 - Live 0xffffffffc0695000
//xt_conntrack 16384 1 - Live 0xffffffffc0690000
//ipt_MASQUERADE 16384 1 - Live 0xffffffffc068b000
//nf_conntrack_netlink 49152 0 - Live 0xffffffffc0679000
//xfrm_user 40960 1 - Live 0xffffffffc066a000
//xfrm_algo 16384 1 xfrm_user, Live 0xffffffffc0651000
//)"); // just some fake data for test
    string line;
    while (getline(ifs, line)) {
        stringstream ss(line);
        string name, memorySize, instanceCount, dependencies, state;
        ss >> name >> memorySize >> instanceCount >> dependencies >> state;
        if (name.empty() || memorySize.empty() || instanceCount.empty() || dependencies.empty() || state.empty()) {
            continue;
        }
        modules.push_back(
            {
                name,
                stoull(memorySize),
                stoull(instanceCount),
                dependencies,
                state
            }
        );
    }
}

vector<Monitor::ModuleInfo> Monitor::getModules() {
    return modules;
}

Monitor::Monitor() {
    readCPUInfo();
    readVersion();
    readHostname();
    readModules();
}

Monitor::~Monitor() = default;
