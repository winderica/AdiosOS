#include <stdio.h>
#include <errno.h>
#include <fcntl.h>
#include <string.h>
#include <unistd.h>

#define BUFFER_LENGTH 256
static char receive[BUFFER_LENGTH];

int main() {
    char message[BUFFER_LENGTH];
    int fd = open("/dev/lkmDevice", O_RDWR);
    if (fd < 0) {
        perror("Failed to open the device");
        return errno;
    }
    printf("Please type your message to the kernel module:\n");
    scanf("%[^\n]%*c", message);
    if (write(fd, message, strlen(message)) < 0) {
        perror("Failed to write the message to the device");
        return errno;
    }
    printf("Sent message: %s\n", message);
    if (read(fd, receive, BUFFER_LENGTH) < 0) {
        perror("Failed to read the message from the device");
        return errno;
    }
    printf("Received message: %s\n", receive);
    close(fd);
    return 0;
}