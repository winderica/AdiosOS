#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <stdarg.h>

void assert(int condition, const char *format, ...) { // simple wrapper for C assert and printf
    if (!condition) {
        va_list args;
        va_start(args, format);
        vprintf(format, args);
        va_end(args);
        exit(EXIT_FAILURE);
    }
}

int openFile(char *filename, int flag) {
    int fd = open(filename, flag);
    assert(fd >= 0, "Failed to open %s\n", filename);
    return fd;
}

void closeFile(char *filename, int fd) {
    assert(close(fd) == 0, "Failed to close %s\n", filename);
}

int main(int argc, char *argv[]) {
    assert(argc == 3, "Usage: %s <source_file> <destination_file>\n", argv[0]);
    char *src = argv[1];
    char *dst = argv[2];
    struct stat st;
    assert(stat(src, &st) == 0, "Failed to get status of %s\n", src);
    int size = st.st_size;
    char content[size];
    int srcFd = openFile(src, O_RDONLY);
    assert(read(srcFd, content, size) == size, "Failed to read from %s\n", src);
    closeFile(src, srcFd);
    int dstFd = creat(dst, 0644);
    assert(dstFd >= 0, "Failed to create %s\n", dst);
    openFile(dst, O_WRONLY);
    assert(write(dstFd, content, size) == size, "Failed to write to %s\n", dst);
    closeFile(dst, dstFd);
    return 0;
}