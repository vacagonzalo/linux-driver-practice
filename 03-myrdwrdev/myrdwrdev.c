#include <linux/module.h>
#include <linux/init.h>
#include <linux/fs.h>
#include <linux/version.h>
#include <linux/device.h>
#include <linux/cdev.h>
#include <linux/uaccess.h>

/* Buffer for data*/
static char buffer[255];
static int buffer_index;


static unsigned int myrdwr_major;
static struct class *myrdwr_class;
static struct cdev myrdwr_cdev;

int myrdwr_open(struct inode * inode, struct file * filp)
{
    pr_info("Someone tried to open me\n");
    return 0;
}

int myrdwr_release(struct inode * inode, struct file * filp)
{
    pr_info("Someone closed me\n");
    return 0;
}

ssize_t myrdwr_read(struct file *filp, char __user * buf, size_t count, loff_t * offset)
{
    int to_copy, not_copied, delta;
    /* Get the amout of data to copy */
    to_copy = min(count, (size_t)buffer_index);

    /* Copy data to user */
    not_copied = copy_to_user(buf, buffer, to_copy);

    /* Calculate data */
    delta = to_copy - not_copied;
    return delta;
}

ssize_t myrdwr_write(struct file * filp, const char __user * buf, size_t count, loff_t * offset)
{
	int to_copy, not_copied, delta;

	/* Get amount of data to copy */
	to_copy = min(count, sizeof(buffer));

	/* Copy data to user */
	not_copied = copy_from_user(buffer, buf, to_copy);
	buffer_index = to_copy;

	/* Calculate data */
	delta = to_copy - not_copied;

	return delta;
}

struct file_operations myrdwr_fops = {
    open:    myrdwr_open,
    release: myrdwr_release,
    read:    myrdwr_read,
    write:   myrdwr_write,
};


static int __init myrdwr_init(void) {
    struct device *myrdwr_device;
    int error;
    dev_t devt = 0;

    /* Get a range of minor numbers */
    error = alloc_chrdev_region(&devt, 0, 1, "myrdwr");
    if (error < 0) {
        pr_err("myrdwr - can't get major number\n");
        return error;
    }
    myrdwr_major = MAJOR(devt);
    pr_info("myrdwr - major number = %d\n", myrdwr_major);

    /* Create device class, visible in /sys/class */
    myrdwr_class = class_create(THIS_MODULE, "myrdwr_class");
    if (IS_ERR(myrdwr_class)) {
        pr_err("myrdwr - Error creating class.\n");
        unregister_chrdev_region(MKDEV(myrdwr_major, 0), 1);
        return PTR_ERR(myrdwr_class);
    }

    /* Initialize the char device and tie a file_operations to it */
    cdev_init(&myrdwr_cdev, &myrdwr_fops);
    myrdwr_cdev.owner = THIS_MODULE;

    /* Make the device live for the users to access */
    cdev_add(&myrdwr_cdev, devt, 1);

    myrdwr_device = device_create(myrdwr_class,
                                     NULL,           /* no parent device */
                                     devt,           /* associated dev_t */
                                     NULL,           /* no additional data */
                                     "myrdwr_char"); /* device name */

    if (IS_ERR(myrdwr_device)) {
        pr_err("myrdwr - error creating myrdwr char device.\n");
        class_destroy(myrdwr_class);
        unregister_chrdev_region(devt, 1);
        return -1;
    }

    pr_info("myrdwr - module loaded\n");
    return 0;
}

static void __exit myrdwr_exit(void) {
    unregister_chrdev_region(MKDEV(myrdwr_major, 0), 1);
    device_destroy(myrdwr_class, MKDEV(myrdwr_major, 0));
    cdev_del(&myrdwr_cdev);
    class_destroy(myrdwr_class);

    pr_info("myrdwr - module unloaded\n");
}

module_init(myrdwr_init);
module_exit(myrdwr_exit);

MODULE_AUTHOR("Gonzalo Nahuel Vaca");
MODULE_DESCRIPTION("Simpre read and write chardev LKM");
MODULE_LICENSE("GPL");
