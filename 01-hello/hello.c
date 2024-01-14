#include<linux/module.h>
#include<linux/init.h>

/* Meta information */
MODULE_LICENSE("GLP");
MODULE_AUTHOR("Gonzalo Nahuel Vaca");
MODULE_DESCRIPTION("Simple hello world LKM");

static int __init hello_init(void) {
    printk("hello_init\n");
    return 0;
}

static void __exit hello_exit(void) {
    printk("hello_exit\n");
}

module_init(hello_init);
module_exit(hello_exit);
