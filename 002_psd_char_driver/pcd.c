#include <linux/module.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/uaccess.h>

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

loff_t pcd_lseek(struct file *fp, loff_t offset, int whence)
{
    loff_t temp;

    pr_info("lseek requested \n");
    pr_info("Current file position is : %lld\n",fp->f_pos);

    switch(whence)
    {
        case SEEK_SET:
            if((offset > DEV_MEM_SIZE) || (offset < 0))
            {
                return -EINVAL;
            }
            fp->f_pos = offset;
            break;
        case SEEK_CUR:
            temp = fp->f_pos + offset;
            if((temp > DEV_MEM_SIZE) || (temp < 0))
            {
                return -EINVAL;
            }
            fp->f_pos = temp;
            break;
        case SEEK_END:
            temp = DEV_MEM_SIZE + offset;
            if((temp > DEV_MEM_SIZE) || (temp < 0))
            {
                return -EINVAL;
            }
            fp->f_pos = temp;
            break;
        default:
            return -EINVAL;
    }

    pr_info("New Value of the file position = %lld\n", fp->f_pos);
    return fp->f_pos;
}

ssize_t pcd_read(struct file *fp, char __user *buff, size_t count, loff_t *f_pos)
{
    pr_info("Read request for %zu bytes\n",count);
    /* Adjust the count */
    if((*f_pos)+count > DEV_MEM_SIZE)
    {
        count = DEV_MEM_SIZE - (*f_pos);
    }
    /* Copy to user */
    if(copy_to_user(buff, &device_buffer[*f_pos], count))
    {
        return -EFAULT;
    }
    /* Update the file postition */
    *f_pos += count;
    pr_info("Number of bytes read = %zu \n",count);
    pr_info("Updated file position = %lld \n",*f_pos);
    return count;
}

ssize_t pcd_write(struct file *fp, const char __user *buff, size_t count, loff_t *f_pos)
{
    pr_info("Write request for %zu bytes\n",count);
    pr_info("Current file position is : %lld\n",fp->f_pos);
    
    /* Adjust the count */
    if((*f_pos) + count > DEV_MEM_SIZE)
    {
        count = DEV_MEM_SIZE - (*f_pos);
    }
    
    if(!count)
    {
        pr_err("Error: No Memory\n");
        return -ENOMEM;
    }
    /* Copy from user */
    if(copy_from_user(&device_buffer[*f_pos],buff,count))
    {
        return -EFAULT;
    }
    /* Update the file position */
    *f_pos += count;
    pr_info("Number of bytes written = %zu \n",count);
    pr_info("Updated file position = %lld \n",*f_pos);
    return count; 
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
    int ret;

	/*1. Dynamically allocate a device number */
    ret = alloc_chrdev_region(&device_number,0,1,"psd");
    if(ret < 0)
    {
        pr_err("Dev allocation failed\n");
        goto out;
    }

	/*2. Initiaize the cdev struct with fops */
	cdev_init(&pcd_cdev, &pcd_fops);

	/*3. Set Owner */
	pcd_cdev.owner = THIS_MODULE;

	/*4. Register Device to VFS */
	ret = cdev_add(&pcd_cdev, device_number, 1);
	if(ret < 0)
	{
	    pr_err("Device add failed \n");
        goto unreg_chrdev;
	}

    /*5 Create Class */
    class_pcd = class_create(THIS_MODULE,"pcd_class");
    if(IS_ERR(class_pcd))
    {
        pr_err("Class creation failed\n");
        ret = PTR_ERR(class_pcd);
        goto cdev_del;
    }

    /*6 Create device and Populate sysfs */
    device_pcd = device_create(class_pcd,NULL,device_number,NULL,"pcd");
    if(IS_ERR(device_pcd))
    {
        pr_err("Device creation failed\n");
        ret = PTR_ERR(device_pcd);
        goto class_del;
    }

    pr_info("PCD Module init was successful \n");
    pr_info("Device Number <major>:<minor>=%d:%d \n",MAJOR(device_number),MINOR(device_number));

    return 0;
    
class_del:
    class_destroy(class_pcd);
cdev_del:
    cdev_del(&pcd_cdev);
unreg_chrdev:
    unregister_chrdev_region(device_number,1);
out:
    return ret;
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
