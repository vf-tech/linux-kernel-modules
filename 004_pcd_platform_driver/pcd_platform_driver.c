#include <linux/module.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/uaccess.h>
#include <linux/platform_device.h>
#include <linux/slab.h>
#include "platform.h"

#define MAX_DEVICES (10)

/* DEVICE ID PRIVATE DATA */
struct pcdev_config_item
{
    uint32_t config_item_1;
    uint32_t config_item_2;
};

enum pcdev_names
{
    PCDEVA1X,
    PCDEVB1X,
    PCDEVC1X,
    PCDEVD1X
};

const struct pcdev_config_item pcdev_config_item_table[] = {
    [PCDEVA1X] = {.config_item_1 = 11, .config_item_2 = 101},
    [PCDEVB1X] = {.config_item_1 = 21, .config_item_2 = 102},
    [PCDEVC1X] = {.config_item_1 = 31, .config_item_2 = 103},
    [PCDEVD1X] = {.config_item_1 = 41, .config_item_2 = 104}
};

const struct platform_device_id pcdev_platform_device_id_table[] = {
    [PCDEVA1X] = {.name = "pcdev-A1x", .driver_data = PCDEVA1X},
    [PCDEVB1X] = {.name = "pcdev-B1x", .driver_data = PCDEVB1X},
    [PCDEVC1X] = {.name = "pcdev-C1x", .driver_data = PCDEVC1X},
    [PCDEVD1X] = {.name = "pcdev-D1x", .driver_data = PCDEVD1X},
    {}, //NULL TERMINATOR
};

/*Device Private data */
struct pcdev_private_data
{
    struct pcdev_platform_data pdata;
    char *buffer;
    dev_t dev_num;
    struct cdev cdev;
};

/*Driver Private Data */
struct pcdrv_private_data
{
    int total_devices;
    dev_t device_number_base;
    struct class *class_pcd;
    struct device *device_pcd;
};

/* Static Driver Data Struct */
struct pcdrv_private_data pcdrv_data;

/* FUNCTIONS */

int check_permission(int dev_perm, int acc_mode)
{

	if(dev_perm == RDWR)
		return 0;

    //ensures readonly access
	if( (dev_perm == RDONLY) && ( (acc_mode & FMODE_READ) && !(acc_mode & FMODE_WRITE) ) )
		return 0;

	//ensures writeonly access
	if( (dev_perm == WRONLY) && ( (acc_mode & FMODE_WRITE) && !(acc_mode & FMODE_READ) ) )
		return 0;

	return -EPERM;

}

loff_t pcd_lseek(struct file *fp, loff_t offset, int whence)
{
    return 0;
}

ssize_t pcd_read(struct file *fp, char __user *buff, size_t count, loff_t *f_pos)
{
    return 0;
}

ssize_t pcd_write(struct file *fp, const char __user *buff, size_t count, loff_t *f_pos)
{
    return -ENOMEM;
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

/* gets called when a matched new device found */
int pcd_platform_driver_probe(struct platform_device *pdev)
{
    int ret;
    struct pcdev_private_data  *dev_data;
    struct pcdev_platform_data *pdata;

    pr_info("A new device found\n");

    /* 1. Get the platform data */
    //Platform data comes from mock, pcd_device_setup
    pdata = (struct pcdev_platform_data*)dev_get_platdata(&pdev->dev);    //pdata = pdev->dev.platform_data;
    if(pdata == NULL)
    {
        pr_info("No Platform Data available\n");
        return -EINVAL;
    }

    /* 2. Dynamically allocate memory for the DEVICE private data */
    dev_data = devm_kzalloc(&pdev->dev, sizeof(*dev_data), GFP_KERNEL);
    if(dev_data == NULL)
    {
        pr_info("No Free RAM Data available\n");
        return -ENOMEM;
    }

    /* Save the device private data in platform device structure */
    dev_set_drvdata(&pdev->dev,dev_data);       //pdev->dev.driver_data = dev_data;

    dev_data->pdata.size = pdata->size;
    dev_data->pdata.perm = pdata->perm;
    dev_data->pdata.serial_number = pdata->serial_number;

    pr_info("Device serial number = %s\n", dev_data->pdata.serial_number);
    pr_info("Device buffer size = %d\n", dev_data->pdata.size);
    pr_info("Device permission = %d\n", dev_data->pdata.perm);

    pr_info("Device config item-1: %d", pcdev_config_item_table[pdev->id_entry->driver_data].config_item_1);
    pr_info("Device config item-2: %d", pcdev_config_item_table[pdev->id_entry->driver_data].config_item_2);

    /* 3. Dynamically allocate memory for the device buffer using platform data*/
    dev_data->buffer = devm_kzalloc(&pdev->dev, dev_data->pdata.size, GFP_KERNEL);
    if(dev_data->buffer == NULL)
    {
        pr_info("Cannot allocate data for data buffer\n");
        return -ENOMEM;
    }

    /* 4. Get the device number */
    dev_data->dev_num = pcdrv_data.device_number_base + pdev->id;

    /* 5. Do cdev init and add */
    /*Init Cdev with fops */
    cdev_init(&dev_data->cdev, &pcd_fops);

    /*Set Owner */
    dev_data->cdev.owner = THIS_MODULE;

    /*Register Device to VFS */
    ret = cdev_add(&dev_data->cdev, dev_data->dev_num, 1);
    if(ret < 0)
    {
        pr_err("C-Device[%d] add failed \n", dev_data->dev_num);
        return ret;
    }
    pr_info("Device Number added <major>:minor=%d:%d\n",MAJOR(dev_data->dev_num),MINOR(dev_data->dev_num));

    /* 6. Create device file for the detected platform device */
    pcdrv_data.device_pcd = device_create(pcdrv_data.class_pcd, NULL, dev_data->dev_num, NULL, "pcdev-%d", pdev->id);
    if(IS_ERR(pcdrv_data.device_pcd))
    {
        pr_err("Device creation failed\n");
        ret = PTR_ERR(pcdrv_data.device_pcd);
        cdev_del(&dev_data->cdev);
        return ret;
    }

    pcdrv_data.total_devices++;

    pr_info("Device probe is succcesfull\n");
    pr_info("Total Device Count: %d\n",pcdrv_data.total_devices);
    /* 7. Error Handling */
    return 0;
}

/* gets called when a device is removed from system */
int pcd_platform_driver_remove(struct platform_device *pdev)
{
    struct pcdev_private_data  *dev_data;

    /* Get the device private data in platform device structure */
    dev_data = dev_get_drvdata(&pdev->dev);                               //dev_data = pdev->dev.driver_data;
    if(dev_data == NULL)
    {
        pr_info("No Device Private Data available\n");
        return -EINVAL;
    }

    /* 1. Remove the device file */
    device_destroy(pcdrv_data.class_pcd, dev_data->dev_num);

    /* 2. Destroy the cdev */
    cdev_del(&dev_data->cdev);

    pcdrv_data.total_devices--;

    pr_info("A device is removed\n");
    pr_info("Total Device Count: %d\n",pcdrv_data.total_devices);
    return 0;
}

/* Platform Driver Struct */
struct platform_driver pcd_platform_driver =
{
    .probe = pcd_platform_driver_probe,
    .remove = pcd_platform_driver_remove,
    .id_table = pcdev_platform_device_id_table,
    .driver = {
        .name = "pseudo-char-device"
        }
};

/* This is module init entry point */
static int __init pcd_platform_driver_init(void)
{
    int ret;
    /* 1. Dynamically allocate device numbers MAX */
    ret = alloc_chrdev_region(&pcdrv_data.device_number_base,0,MAX_DEVICES,"pcdevs");
    if(ret < 0)
    {
        pr_err("Alloc chrdev failed\n");
        return ret;
    }

    /* 2. Create Device class under /sys/class/ */
    pcdrv_data.class_pcd = class_create(THIS_MODULE, "pcd_class");
    if(IS_ERR(pcdrv_data.class_pcd))
    {
        pr_err("Class creation failed\n");
        ret = PTR_ERR(pcdrv_data.class_pcd);
        unregister_chrdev_region(pcdrv_data.device_number_base,MAX_DEVICES);
        return ret;
    }

    /* 3. Register the platform driver */
    platform_driver_register(&pcd_platform_driver);

    pr_info("pcd platform driver loaded\n");
    return 0;
}

/* This is module exit entry point */
static void __exit pcd_platform_driver_cleanup(void)
{
    /* 1. Unregister the platform driver*/
    platform_driver_unregister(&pcd_platform_driver);

    /* 2. Destroy Device Class */
    class_destroy(pcdrv_data.class_pcd);

    /* 3. Free Allocated Device Numbers */
    unregister_chrdev_region(pcdrv_data.device_number_base,MAX_DEVICES);

    pr_info("pcd platform driver unloaded\n");
}

/* Registration Points */
module_init(pcd_platform_driver_init);
module_exit(pcd_platform_driver_cleanup);

/* Description of module */
MODULE_LICENSE("GPL");
MODULE_AUTHOR("Mehmet ALINBAY");
MODULE_DESCRIPTION("A Simple Character Device Kernel Module");
