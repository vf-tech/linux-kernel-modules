#include <linux/module.h>
#include <linux/fs.h>
#include <linux/cdev.h>

#define DEV_MEM_SIZE 512

/* device memory */
char device_buffer[DEV_MEM_SIZE];

/* This holds the device number */
dev_t device_number;

/* Cdev variable */
struct cdev pcd_cdev;

/* Fops Struct */
struct file_operations pcd_fops;

/* This is module init entry point */
static int __init psd_driver_init(void)
{
	/*1. Dynamically allocate a device number */
	alloc_chrdev_region(&device_number,0,1,"psd");

	/*2. Initiaize the cdev struct with fops */
	cdev_init(&pcd_cdev, &pcd_fops);

	/*3. Set Owner */
	pcd_cdev.owner = THIS_MODULE;

	/*4. Register Device to VFS */
	cdev_add(&pcd_cdev, device_number, 1);

    return 0;
}

/* Thisi is module exit entry point */
static void __exit psd_driver_exit(void)
{
}

/* Registration Points */
module_init(psd_driver_init);
module_exit(psd_driver_exit);

/* Description of module */
MODULE_LICENSE("GPL");
MODULE_AUTHOR("Mehmet ALINBAY");
MODULE_DESCRIPTION("A Simple Character Device Kernel Module");
