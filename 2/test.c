#include <linux/kernel.h>
#include <unistd.h>
#include <string.h>

int main(int argc, char *argv[]) {
    return syscall(556, argv[1], strlen(argv[1]), argv[2], strlen(argv[2]));
}

// qemu-system-i386 -kernel /build/arch/x86/boot/bzImage -initrd /a.cpio.gz -nographic -append "console=ttyS0"