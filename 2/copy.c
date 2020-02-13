#include <linux/kernel.h>
#include <linux/syscalls.h>
#include <linux/slab.h>

SYSCALL_DEFINE4(copy, char*, src, int, srcLength, char *, dst, int, dstLength) {
    struct kstat st;
    unsigned int size;
    mm_segment_t originalFs;
    int status;
    long srcFd;
    long dstFd;
    char *srcInternal;
    char *dstInternal;
    char *content;
    srcInternal = kmalloc(srcLength + 1, GFP_KERNEL);
    dstInternal = kmalloc(dstLength + 1, GFP_KERNEL);
    if (srcInternal == NULL || dstInternal == NULL) {
        return -ENOMEM;
    }
    if (strncpy_from_user(srcInternal, src, srcLength + 1) != srcLength) {
        printk("Failed to copy src filename into kernel.\n");
        kfree(srcInternal);
        return -EFAULT;
    }
    if (strncpy_from_user(dstInternal, dst, dstLength + 1) != dstLength) {
        printk("Failed to copy dst filename into kernel.\n");
        kfree(dstInternal);
        return -EFAULT;
    }

    originalFs = get_fs();
    set_fs(KERNEL_DS);

    srcFd = ksys_open(srcInternal, O_RDONLY, 0644);
    if (srcFd < 0) {
        printk("Failed to open src.\n");
        set_fs(originalFs);

        kfree(srcInternal);
        kfree(dstInternal);
        return srcFd;
    }
    status = vfs_fstat(srcFd, &st);
    if (status < 0) {
        printk("Failed to get status of src.\n");
        ksys_close(srcFd);
        set_fs(originalFs);

        kfree(srcInternal);
        kfree(dstInternal);
        return status;
    }
    size = st.size;
    content = kmalloc(size + 1, GFP_KERNEL);
    ksys_read(srcFd, content, size);
    content[size] = '\0';
    printk("Content of src: %s\n", content);
    ksys_close(srcFd);

    dstFd = ksys_open(dstInternal, O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if (dstFd < 0) {
        printk("Failed to create/open dst.\n");
        ksys_close(dstFd);
        set_fs(originalFs);

        kfree(srcInternal);
        kfree(dstInternal);
        kfree(content);
        return dstFd;
    }
    ksys_write(dstFd, content, size);
    ksys_close(dstFd);

    set_fs(originalFs);

    kfree(srcInternal);
    kfree(dstInternal);
    kfree(content);
    return 0;
}
