#include <linux/init.h>
#include <linux/module.h>
#include <linux/device.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/uaccess.h>
#include <linux/mutex.h>

#define  CLASS_NAME  "lkmClass"
#define  DEVICE_NAME "lkmDevice"

MODULE_LICENSE("GPL");
static DEFINE_MUTEX(deviceMutex);
static int majorNumber;
static const int minorNumber = 0;
static char message[256] = {0};
static short messageSize;
static struct class *driverClass = NULL;
static struct device *driverDevice = NULL;

static int deviceOpen(struct inode *, struct file *);

static int deviceRelease(struct inode *, struct file *);

static ssize_t deviceRead(struct file *, char *, size_t, loff_t *);

static ssize_t deviceWrite(struct file *, const char *, size_t, loff_t *);

static struct file_operations operations = {
        .open = deviceOpen,
        .read = deviceRead,
        .write = deviceWrite,
        .release = deviceRelease
};

static int __init moduleInit(void) {
    majorNumber = register_chrdev(0, DEVICE_NAME, &operations);
    if (majorNumber < 0) {
        printk(KERN_ALERT "Failed to register a major number.\n");
        return majorNumber;
    }
    driverClass = class_create(THIS_MODULE, CLASS_NAME);
    if (IS_ERR(driverClass)) {
        unregister_chrdev(majorNumber, DEVICE_NAME);
        printk(KERN_ALERT "Failed to register device class.\n");
        return PTR_ERR(driverClass);
    }
    driverDevice = device_create(driverClass, NULL, MKDEV(majorNumber, minorNumber), NULL, DEVICE_NAME);
    if (IS_ERR(driverDevice)) {
        class_destroy(driverClass);
        unregister_chrdev(majorNumber, DEVICE_NAME);
        printk(KERN_ALERT "Failed to create the device.\n");
        return PTR_ERR(driverDevice);
    }
    mutex_init(&deviceMutex);
    printk(KERN_INFO "LKM initialized.\n");
    return 0;
}

static void __exit moduleExit(void) {
    mutex_destroy(&deviceMutex);
    device_destroy(driverClass, MKDEV(majorNumber, minorNumber));
    class_unregister(driverClass);
    class_destroy(driverClass);
    unregister_chrdev(majorNumber, DEVICE_NAME);
    printk(KERN_INFO "LKM exited.\n");
}

static int deviceOpen(struct inode *inode, struct file *fp) {
    if (!mutex_trylock(&deviceMutex)) {
        printk(KERN_ALERT "Device in use by another process.\n");
        return -EBUSY;
    }
    printk(KERN_INFO "Device opened.\n");
    return 0;
}

static int deviceRelease(struct inode *inode, struct file *fp) {
    mutex_unlock(&deviceMutex);
    printk(KERN_INFO "Device released.\n");
    return 0;
}

static ssize_t deviceRead(struct file *fp, char *buffer, size_t len, loff_t *offset) {
    if (copy_to_user(buffer, message, messageSize)) {
        printk(KERN_ALERT "Failed to send message to user.\n");
        return -EFAULT;
    }
    messageSize = 0;
    return 0;
}

static ssize_t deviceWrite(struct file *fp, const char *buffer, size_t len, loff_t *offset) {
    if (copy_from_user(message, buffer, len)) {
        printk(KERN_ALERT "Failed to read message from user.\n");
        return -EFAULT;
    }
    messageSize = len;
    return messageSize;
}

module_init(moduleInit);
module_exit(moduleExit);
