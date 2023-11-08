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

/* Class variable */
struct class *class_pcd;

/* Device variable */
struct device *device_pcd;

loff_t pcd_lseek(struct file *fp, loff_t off, int whence)
{
    return 0;
}

ssize_t pcd_read(struct file *fp, char __user *buff, size_t count, loff_t *f_pos)
{
    return 0;
}

ssize_t pcd_write(struct file *fp, const char __user *buff, size_t count, loff_t *f_pos)
{
    return 0;
}

int pcd_open(struct inode *inode, struct file *fp)
{
    return 0;
}

int pcd_release(struct inode *inode, struct file *fp)
{
    return 0;
}

/* Fops Struct */
const struct file_operations pcd_fops = 
{
    .open = pcd_open,
    .write = pcd_write,
    .read = pcd_read,
    .llseek = pcd_lseek,
    .release = pcd_release,
    .owner = THIS_MODULE
};


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

    /*5 Create Class */
    class_pcd = class_create(THIS_MODULE,"pcd_class");

    /*6 Create device and Populate sysfs */
    device_pcd = device_create(class_pcd,NULL,device_number,NULL,"pcd");

    pr_info("PCD Module init was successful\n");
    pr_info("Device Number <major>:<minor>=%d:%d\n",MAJOR(device_number),MINOR(device_number));

    return 0;
}

/* This is module exit entry point */
static void __exit psd_driver_exit(void)
{
    /*1 Destroy Device */
    device_destroy(class_pcd,device_number);

    /*2 Destroy Class */
    class_destroy(class_pcd);

    /*3 Delete CDEV*/
    cdev_del(&pcd_cdev);

    /*4 Unregister */
    unregister_chrdev_region(device_number,1);
    
    pr_info("PCD Module cleanup, done\n"); 
    
}

/* Registration Points */
module_init(psd_driver_init);
module_exit(psd_driver_exit);

/* Description of module */
MODULE_LICENSE("GPL");
MODULE_AUTHOR("Mehmet ALINBAY");
MODULE_DESCRIPTION("A Simple Character Device Kernel Module");
