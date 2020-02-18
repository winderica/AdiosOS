#ifndef _MONITOR_H
#define _MONITOR_H

#include <string>
#include <vector>
#include <map>
#include <unordered_map>

using namespace std;
using namespace chrono;

class Monitor {
public:
    using ull = unsigned long long;
    using is_iter = istreambuf_iterator<char>;
    struct CPUInfo {
        string vendorId;
        string modelName;
        string frequency;
        string cache;
        int processors;
        int cores;
    };

    struct CPUJiffies {
        ull user;
        ull nice;
        ull system;
        ull idle;
        ull iowait;
        ull irq;
        ull softirq;

        inline ull busy() { return user + nice + system + irq + softirq; }

        inline ull total() { return busy() + idle + iowait; }
    };

    struct MemoryInfo {
        ull totalVirtual;
        ull usedVirtual;
        ull totalPhysical;
        ull usedPhysical;
    };

    struct ProcessInfo {
        ull pid;
        ull uid;
        ull parentPid;
        ull virtualMemory;
        ull physicalMemory;
        ull sharedMemory;
        ull priority;
        ull nice;
        double memoryUsage;
        double cpuUsage;
        duration<double> time;
        string name;
        string user;
        string command;
        string state;
    };

    struct Usage {
        map<ull, ProcessInfo> processes;
        CPUJiffies currentJiffies;
        double cpuUsage;
    };

    struct TimeInfo {
        duration<double> upTime;
        duration<double> idleTime;
        time_point<system_clock> bootTime;
    };

    struct ModuleInfo {
        string name;
        ull memorySize;
        ull instanceCount;
        string dependencies;
        string state;
    };
private:
    unordered_map<ull, string> users; // uid -> username
    vector<ModuleInfo> modules;
    Usage usage;
    CPUInfo cpuInfo;
    string version;
    string hostname;

    void readJiffies();

    void readVersion();

    void readHostname();

    void readCPUInfo();

    void readModules();

    static pair<double, double> readUpTime();

    static vector<string> split(const string &str, const string &delimiter);

    static inline ull parseLine(const string &data);

public:
    Monitor();

    ~Monitor();

    string getVersion();

    string getHostname();

    Monitor::CPUInfo getCPUInfo();

    Usage getUsage();

    MemoryInfo getMemoryInfo();

    TimeInfo getCPUTime();

    vector<ModuleInfo> getModules();
};

#endif // _MONITOR_H
