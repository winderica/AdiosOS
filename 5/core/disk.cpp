#include "disk.h"

#include <fcntl.h>
#include <unistd.h>
#include <filesystem>

Disk::Disk(const char *path) : _mounted(false) {
    fd = open(path, O_RDWR | O_CREAT, 0600);
    if (fd < 0) {
        throw runtime_error("Unable to open disk");
    }
    blocks = filesystem::file_size(path) / BLOCK_SIZE;
}

Disk::~Disk() {
    if (fd > 0) {
        close(fd);
        fd = 0;
    }
}

void Disk::checkParams(unsigned int index, const char *data) {
    if (index >= blocks) {
        throw runtime_error("Invalid block index");
    }

    if (data == nullptr) {
        throw runtime_error("Null data pointer");
    }
}

void Disk::read(unsigned int index, char *data) {
    checkParams(index, data);

    if (lseek(fd, index * BLOCK_SIZE, SEEK_SET) < 0) {
        throw runtime_error("Unable to lseek block " + to_string(index));
    }

    if (::read(fd, data, BLOCK_SIZE) != BLOCK_SIZE) {
        throw runtime_error("Unable to read block " + to_string(index));
    }
}

void Disk::write(unsigned int index, char *data) {
    checkParams(index, data);

    if (lseek(fd, index * BLOCK_SIZE, SEEK_SET) < 0) {
        throw runtime_error("Unable to lseek block " + to_string(index));
    }

    if (::write(fd, data, BLOCK_SIZE) != BLOCK_SIZE) {
        throw runtime_error("Unable to write block " + to_string(index));
    }
}

void Disk::mount() {
    if (_mounted) {
        throw runtime_error("Disk has already been mounted.");
    }
    _mounted = true;
}

void Disk::unmount() {
    if (!_mounted) {
        throw runtime_error("Disk is not mounted");
    }
    _mounted = false;
}
