#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/semaphore.h>
#include <asm/uaccess.h>

#define DEVICE_NAME "cforce"

struct Device {
    char data[100];
    struct semaphore sem;
} device;

static int dev_open(struct inode* inode, struct file* filep);
static int dev_close(struct inode* inode, struct file* filep);
static ssize_t dev_write(struct file* filep, const char* buffer, size_t count, loff_t* offset);
static ssize_t dev_read(struct file* filep, char* buffer, size_t count, loff_t* offset);

struct file_operations fops = {
    .owner = THIS_MODULE,
    .open = dev_open,
    .release = dev_close,
    .write = dev_write,
    .read = dev_read
};

struct cdev* mcdev;
dev_t dev_id;
int major_number;
int ret;

static int __init driver_entry(void)
{
    printk(KERN_ALERT "%s: LOADED\n", DEVICE_NAME);

    // Dynamically Allocate Device and Generate Major Number
    ret = alloc_chrdev_region(&dev_id, 0, 1, DEVICE_NAME);
    if(ret < 0) {
        printk(KERN_ALERT "%s: Failed to Allocate Major Number\n", DEVICE_NAME);
        return ret;
    }

    major_number = MAJOR(dev_id);
	printk(KERN_INFO "%s: Allocated Major Number %d\n", DEVICE_NAME, major_number);
	printk(KERN_INFO "%s: Create Device File \"mknod /dev/%s c %d 0\"\n", DEVICE_NAME, DEVICE_NAME, major_number);

    // Initalize Char Device
    mcdev = cdev_alloc();
    mcdev->owner = THIS_MODULE;
    mcdev->ops = &fops;

    // Add Char Device to Kernel
    ret = cdev_add(mcdev, dev_id, 1);
    if(ret < 0) {
        printk(KERN_ALERT "%s: Unable to Add the Char Device to Kernel\n", DEVICE_NAME);
		return ret;
    }
    
    // Initalize Semaphore
    sema_init(&device.sem, 1);

    return 0;
}

static void __exit driver_exit(void)
{
    // Delete Char Device from Kernel And Unregister Driver
    cdev_del(mcdev);
    unregister_chrdev_region(dev_id, 1);
    printk(KERN_ALERT "%s: UNLOADED\n", DEVICE_NAME);
}

static int dev_open(struct inode* inode, struct file* filep)
{
    if(down_interruptible(&device.sem) != 0) {
        printk(KERN_ALERT "%s: Could not lock the device during open\n", DEVICE_NAME);
        return -1;
    }

    printk(KERN_INFO "%s: Opened Device\n", DEVICE_NAME);
    return 0;
}

static int dev_close(struct inode* inode, struct file* filep)
{
    up(&device.sem);
    printk(KERN_INFO "%s: Device Closed\n", DEVICE_NAME);
    return 0;
}

static ssize_t dev_write(struct file* filep, const char* buffer, size_t count, loff_t* offset)
{
    printk(KERN_INFO "%s: Writing to Device\n", DEVICE_NAME);
    return copy_from_user(device.data, buffer, count);
}

static ssize_t dev_read(struct file* filep, char* buffer, size_t count, loff_t* offset)
{
    printk(KERN_INFO "%s: Reading from Device\n", DEVICE_NAME);
    return copy_to_user(buffer, device.data, count);
}

module_init(driver_entry);
module_exit(driver_exit);
MODULE_LICENSE("GPL");
MODULE_AUTHOR("General Secretary");
MODULE_DESCRIPTION("CForce Char Driver");