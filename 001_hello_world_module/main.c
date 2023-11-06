#include <linux/module.h>

/* This is module init entry point */
static int __init my_kernel_module_init(void)
{
    pr_info("Hello World\n");
    return 0;
}

/* Thisi is module exit entry point */
static void __exit my_kernel_module_exit(void)
{
    pr_info("Goodbye World\n");
}

/* Registration Points */
module_init(my_kernel_module_init);
module_exit(my_kernel_module_exit);

/* Description of module */
MODULE_LICENSE("GPL");
MODULE_AUTHOR("Mehmet ALINBAY");
MODULE_DESCRIPTION("A Simple Kernel Module that Prints Hello World");


