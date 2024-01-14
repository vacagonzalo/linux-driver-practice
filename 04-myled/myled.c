#include <linux/module.h>
#include <linux/init.h>
#include <linux/fs.h>
#include <linux/version.h>
#include <linux/device.h>
#include <linux/cdev.h>
#include <linux/uaccess.h>
#include <linux/gpio.h>

static unsigned int myled_major;
static struct class *myled_class;
static struct cdev myled_cdev;

int myled_open(struct inode * inode, struct file * filp)
{
    pr_info("Someone tried to open me\n");
    return 0;
}

int myled_release(struct inode * inode, struct file * filp)
{
    pr_info("Someone closed me\n");
    return 0;
}

ssize_t myled_read(struct file *filp, char __user * buf, size_t count, loff_t * offset)
{
    int to_copy, not_copied, delta;
    char tmp[3] = " \n";

    /* Get the amout of data to copy */
    to_copy = min(count, sizeof(tmp));

    /* Read value of button */
	printk("Value of button: %d\n", gpio_get_value(17));
	tmp[0] = gpio_get_value(17) + '0';

    /* Copy data to user */
    not_copied = copy_to_user(buf, &tmp, to_copy);

    /* Calculate data */
    delta = to_copy - not_copied;
    return delta;
}

ssize_t myled_write(struct file * filp, const char __user * buf, size_t count, loff_t * offset)
{
	int to_copy, not_copied, delta;
    char value;

	/* Get amount of data to copy */
	to_copy = min(count, sizeof(value));

	/* Copy data to user */
	not_copied = copy_from_user(&value, buf, to_copy);

    /* Setting the LED */
	switch(value) {
		case '0':
			gpio_set_value(23, 0);
			break;
		case '1':
			gpio_set_value(23, 1);
			break;
        case '\n': /* Ignore new line */
            break;
		default:
			printk("Invalid input - %c - %c\n", value, value + '0');
			break;
	}
	/* Calculate data */
	delta = to_copy - not_copied;

	return delta;
}

struct file_operations myled_fops = {
    open:    myled_open,
    release: myled_release,
    read:    myled_read,
    write:   myled_write,
};


static int __init myled_init(void) {
    struct device *myled_device;
    int error;
    dev_t devt = 0;

    /* Get a range of minor numbers */
    error = alloc_chrdev_region(&devt, 0, 1, "myled");
    if (error < 0) {
        pr_err("myled - can't get major number\n");
        return error;
    }
    myled_major = MAJOR(devt);
    pr_info("myled - major number = %d\n", myled_major);

    /* Create device class, visible in /sys/class */
    myled_class = class_create(THIS_MODULE, "myled_class");
    if (IS_ERR(myled_class)) {
        pr_err("myled - Error creating class.\n");
        goto myled_init_class_error;
    }

    /* Initialize the char device and tie a file_operations to it */
    cdev_init(&myled_cdev, &myled_fops);
    myled_cdev.owner = THIS_MODULE;

    /* Make the device live for the users to access */
    cdev_add(&myled_cdev, devt, 1);

    myled_device = device_create(myled_class,
                                     NULL,           /* no parent device */
                                     devt,           /* associated dev_t */
                                     NULL,           /* no additional data */
                                     "myled_char"); /* device name */

    if (IS_ERR(myled_device)) {
        pr_err("myled - error creating myled char device.\n");
        goto myled_init_file_error;
    }

    /* GPIO 23 init */
	if(gpio_request(23, "rpi-gpio-23")) {
		printk("Can not allocate GPIO 23\n");
        goto myled_init_gpio_error;
	}

	/* Set GPIO 23 direction */
	if(gpio_direction_output(23, 0)) {
		printk("Can not set GPIO 23 to output!\n");
        goto myled_init_gpio_error;
	}

    pr_info("myled - module loaded\n");
    return 0;
    
myled_init_gpio_error:
    gpio_free(23);
myled_init_file_error:
    class_destroy(myled_class);
myled_init_class_error:
    unregister_chrdev_region(MKDEV(myled_major, 0), 1);
    return -1;

}

static void __exit myled_exit(void) {
    gpio_set_value(23, 0);
	gpio_free(17);
	gpio_free(23);
    unregister_chrdev_region(MKDEV(myled_major, 0), 1);
    device_destroy(myled_class, MKDEV(myled_major, 0));
    cdev_del(&myled_cdev);
    class_destroy(myled_class);

    pr_info("myled - module unloaded\n");
}

module_init(myled_init);
module_exit(myled_exit);

MODULE_AUTHOR("Gonzalo Nahuel Vaca");
MODULE_DESCRIPTION("Simpre read and write chardev LKM");
MODULE_LICENSE("GPL");
