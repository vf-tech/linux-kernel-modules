#include <linux/module.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/uaccess.h>

#define RDONLY 	(0x01)
#define WRONLY 	(0x02)
#define RDWR	(0x03)

#define NO_OF_DEVICES 4

#define DEV_MEM_SIZE_PCDEV1 1024
#define DEV_MEM_SIZE_PCDEV2 512
#define DEV_MEM_SIZE_PCDEV3 1024
#define DEV_MEM_SIZE_PCDEV4 512

/* device memory */
char device_buffer_pcdev1[DEV_MEM_SIZE_PCDEV1];
char device_buffer_pcdev2[DEV_MEM_SIZE_PCDEV2];
char device_buffer_pcdev3[DEV_MEM_SIZE_PCDEV3];
char device_buffer_pcdev4[DEV_MEM_SIZE_PCDEV4];

/*Device Private data */
struct pcdev_priv_data
{
    char *buffer;
    unsigned size;
    const char *serial_number;
    int perm;
    struct cdev cdev;
};

/*Driver Private Data */
struct pcdrv_private_data
{
    int total_devices;
    dev_t device_number;
    struct class *class_pcd;
    struct device *device_pcd;
    struct pcdev_priv_data pcdev_data[NO_OF_DEVICES];
};

struct pcdrv_private_data pcdrv =
{
    .total_devices = NO_OF_DEVICES,
    .pcdev_data =
    {
        [0] =
        {
            .buffer = device_buffer_pcdev1,
            .size = DEV_MEM_SIZE_PCDEV1,
            .serial_number = "PCDEV_0001",
            .perm = RDONLY,
        },
        [1] =
        {
            .buffer = device_buffer_pcdev2,
            .size = DEV_MEM_SIZE_PCDEV2,
            .serial_number = "PCDEV_0002",
            .perm = WRONLY,
        },
        [2] =
        {
            .buffer = device_buffer_pcdev3,
            .size = DEV_MEM_SIZE_PCDEV3,
            .serial_number = "PCDEV_0003",
            .perm = RDWR,
        },
        [3] =
        {
            .buffer = device_buffer_pcdev4,
            .size = DEV_MEM_SIZE_PCDEV4,
            .serial_number = "PCDEV_0004",
            .perm = RDWR,
        },

    },
};

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
    struct pcdev_priv_data *pcdev_data = (struct pcdev_priv_data *)fp->private_data;
    int max_size = pcdev_data->size;
    loff_t temp;

    pr_info("lseek requested \n");
    pr_info("Current file position is : %lld\n",fp->f_pos);

    switch(whence)
    {
        case SEEK_SET:
            if((offset > max_size) || (offset < 0))
            {
                return -EINVAL;
            }
            fp->f_pos = offset;
            break;
        case SEEK_CUR:
            temp = fp->f_pos + offset;
            if((temp > max_size) || (temp < 0))
            {
                return -EINVAL;
            }
            fp->f_pos = temp;
            break;
        case SEEK_END:
            temp = max_size + offset;
            if((temp > max_size) || (temp < 0))
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
    struct pcdev_priv_data *pcdev_data = (struct pcdev_priv_data *)fp->private_data;
    int max_size = pcdev_data->size;
    pr_info("Read request for %zu bytes\n",count);

    /* Adjust the count */
    if((*f_pos)+count > max_size)
    {
        count = max_size - (*f_pos);
    }
    /* Copy to user */
    if(copy_to_user(buff, &pcdev_data->buffer[*f_pos], count))
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
    struct pcdev_priv_data *pcdev_data = (struct pcdev_priv_data *)fp->private_data;
    int max_size = pcdev_data->size;

    pr_info("Write request for %zu bytes\n",count);
    pr_info("Current file position is : %lld\n",fp->f_pos);
    /* Adjust the count */
    if((*f_pos) + count > max_size)
    {
        count = max_size - (*f_pos);
    }

    if(!count)
    {
        pr_err("Error: No Memory\n");
        return -ENOMEM;
    }
    /* Copy from user */
    if(copy_from_user(&pcdev_data->buffer[*f_pos],buff,count))
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
    int ret;
    int minor_n;
    struct pcdev_priv_data *pcdev_data;

    /* Get the minor value */
    minor_n = MINOR(inode->i_rdev);
    pr_info("Minor access = %d\n",minor_n);

    /* Get the data structure */
    pcdev_data = container_of(inode->i_cdev, struct pcdev_priv_data, cdev);

    /* Set the file pointer to minor private data */
    fp->private_data = pcdev_data;

    /* Check Permission */
    ret = check_permission(pcdev_data->perm,fp->f_mode);

    if(!ret)
    {
        pr_info("Device Open is succesfull\n");
    }
    else
    {
        pr_err("Device open is failed\n");
    }
    return ret;
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
    int counter;

	/*Dynamically allocate a device number */
    ret = alloc_chrdev_region(&pcdrv.device_number, 0, NO_OF_DEVICES, "pcd_multi");
    if(ret < 0)
    {
        pr_err("Dev allocation failed\n");
        goto out;
    }

    /*Create Class */
    pcdrv.class_pcd = class_create(THIS_MODULE, "pcd_class");
    if(IS_ERR(pcdrv.class_pcd))
    {
        pr_err("Class creation failed\n");
        ret = PTR_ERR(pcdrv.class_pcd);
        goto unreg_chrdev;
    }

	/*Initiaize the cdev struct with fops */
	for(counter = 0; counter < NO_OF_DEVICES; counter++)
	{
        /*Init Cdev with fops */
	    cdev_init(&pcdrv.pcdev_data[counter].cdev, &pcd_fops);

	    /*Set Owner */
	    pcdrv.pcdev_data[counter].cdev.owner = THIS_MODULE;

	    /*Register Device to VFS */
	    ret = cdev_add(&pcdrv.pcdev_data[counter].cdev, pcdrv.device_number+counter, 1);
	    if(ret < 0)
	    {
	        pr_err("C-Device[%d] add failed \n",counter);
            goto cdev_del;
	    }
	    pr_info("Device Number added <major>:minor=%d:%d\n",MAJOR(pcdrv.device_number+counter),MINOR(pcdrv.device_number+counter));

	    /*Create device and Populate sysfs */
        pcdrv.device_pcd = device_create(pcdrv.class_pcd, NULL, pcdrv.device_number+counter, NULL, "pcdev-%d", counter);
        if(IS_ERR(pcdrv.device_pcd))
        {
            pr_err("Device creation failed\n");
            ret = PTR_ERR(pcdrv.device_pcd);
            goto class_del;
        }

    }

    pr_info("PCD Module init was successful \n");
    pr_info("Device Number <major>:<minor>=%d:%d \n",MAJOR(pcdrv.device_number),MINOR(pcdrv.device_number));

    return 0;

class_del:
cdev_del:
    for(; counter>=0; counter--)
    {
        device_destroy(pcdrv.class_pcd, pcdrv.device_number+counter);
        cdev_del(&pcdrv.pcdev_data[counter].cdev);
    }
    class_destroy(pcdrv.class_pcd);
unreg_chrdev:
    unregister_chrdev_region(pcdrv.device_number,NO_OF_DEVICES);
out:
    return ret;
}

/* This is module exit entry point */
static void __exit psd_driver_exit(void)
{
    int counter;

    for(counter = 0; counter < NO_OF_DEVICES; counter++)
    {
        /*Destroy Device */
        device_destroy(pcdrv.class_pcd, pcdrv.device_number+counter);
        /*Delete CDEV*/
        cdev_del(&pcdrv.pcdev_data[counter].cdev);
    }

    /*2 Destroy Class */
    class_destroy(pcdrv.class_pcd);

    /*4 Unregister */
    unregister_chrdev_region(pcdrv.device_number,NO_OF_DEVICES);

    pr_info("PCD Module cleanup, done\n");
}

/* Registration Points */
module_init(psd_driver_init);
module_exit(psd_driver_exit);

/* Description of module */
MODULE_LICENSE("GPL");
MODULE_AUTHOR("Mehmet ALINBAY");
MODULE_DESCRIPTION("A Simple Character Device Kernel Module");
