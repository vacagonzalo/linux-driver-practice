#include <linux/module.h>
#include <linux/init.h>
#include <linux/fs.h>
#include <linux/version.h>
#include <linux/device.h>
#include <linux/cdev.h>

static unsigned int mychardev_major;
static struct class *mychardev_class;
static struct cdev mychardev_cdev;

int mychardev_open(struct inode * inode, struct file * filp)
{
    pr_info("Someone tried to open me\n");
    return 0;
}

int mychardev_release(struct inode * inode, struct file * filp)
{
    pr_info("Someone closed me\n");
    return 0;
}

ssize_t mychardev_read (struct file *filp, char __user * buf, size_t count,
                                loff_t * offset)
{
    pr_info("Nothing to read\n");
    return 0;
}

ssize_t mychardev_write(struct file * filp, const char __user * buf, size_t count,
                                loff_t * offset)
{
    pr_info("Can't accept any data\n");
    return count;
}

struct file_operations mychardev_fops = {
    open:    mychardev_open,
    release: mychardev_release,
    read:    mychardev_read,
    write:   mychardev_write,
};


static int __init mychardev_init(void) {
    struct device *mychardev_device;
    int error;
    dev_t devt = 0;

    /* Get a range of minor numbers */
    error = alloc_chrdev_region(&devt, 0, 1, "mychardev");
    if (error < 0) {
        pr_err("mychardev - can't get major number\n");
        return error;
    }
    mychardev_major = MAJOR(devt);
    pr_info("mychardev - major number = %d\n", mychardev_major);

    /* Create device class, visible in /sys/class */
    mychardev_class = class_create(THIS_MODULE, "mychardev_class");
    if (IS_ERR(mychardev_class)) {
        pr_err("mychardev - Error creating class.\n");
        unregister_chrdev_region(MKDEV(mychardev_major, 0), 1);
        return PTR_ERR(mychardev_class);
    }

    /* Initialize the char device and tie a file_operations to it */
    cdev_init(&mychardev_cdev, &mychardev_fops);
    mychardev_cdev.owner = THIS_MODULE;

    /* Make the device live for the users to access */
    cdev_add(&mychardev_cdev, devt, 1);

    mychardev_device = device_create(mychardev_class,
                                     NULL,              /* no parent device */
                                     devt,              /* associated dev_t */
                                     NULL,              /* no additional data */
                                     "mychardev_char"); /* device name */

    if (IS_ERR(mychardev_device)) {
        pr_err("mychardev - error creating mychardev char device.\n");
        class_destroy(mychardev_class);
        unregister_chrdev_region(devt, 1);
        return -1;
    }

    pr_info("mychardev - module loaded\n");
    return 0;
}

static void __exit mychardev_exit(void) {
    unregister_chrdev_region(MKDEV(mychardev_major, 0), 1);
    device_destroy(mychardev_class, MKDEV(mychardev_major, 0));
    cdev_del(&mychardev_cdev);
    class_destroy(mychardev_class);

    pr_info("mychardev - module unloaded\n");
}

module_init(mychardev_init);
module_exit(mychardev_exit);

MODULE_AUTHOR("Gonzalo Nahuel Vaca");
MODULE_DESCRIPTION("Simple devchar LKM");
MODULE_LICENSE("GPL");
