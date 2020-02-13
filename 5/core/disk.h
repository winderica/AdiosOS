#ifndef _DISK_H
#define _DISK_H

#include <stdexcept>
#include <cstring>

using namespace std;

class Disk {
private:
    int fd;
    bool _mounted;
    size_t blocks;

    void checkParams(unsigned int index, const char *data);

public:
    const static size_t BLOCK_SIZE = 4096;

    explicit Disk(const char *path);

    ~Disk();

    [[nodiscard]] size_t size() const { return blocks; }

    [[nodiscard]] bool mounted() const { return _mounted; }

    void mount();

    void unmount();

    void read(unsigned int index, char *data);

    void write(unsigned int index, char *data);
};

#endif // _DISK_H
