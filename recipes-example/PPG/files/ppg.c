/*
    A Linux character-based driver (cDD) used to access a “virtual” Photopletismography (PPG) sensor
*/

#include <linux/module.h>
#include <linux/init.h>
#include <linux/kernel.h>

#include <linux/types.h>
#include <linux/kdev_t.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/device.h>
#include <linux/uaccess.h>

#include "data.h"

static dev_t PPG_dev;

struct cdev PPG_cdev;

struct class *myclass = NULL;

static char buffer[64];

// index for value to read from static data array
static uint16_t i;

ssize_t PPG_read(struct file *filp, char __user *buf, size_t count, loff_t *f_pos)
{
    //  Copy a block of data into user space
    if (copy_to_user(buf, &data[i++], sizeof(&data[0])) == 0)
        printk(KERN_INFO "[PPG] read (position=%d, data=%d)\n", i, *((int *)buf));
    else
        printk(KERN_INFO "[PPG] error to read (position=%d, data=%d,)\n", i, *((int *)buf));

    // if read all value, start from zero again
    if (i == 2048) 
        i = 0;

    return count;
}

int PPG_open(struct inode *inode, struct file *filp)
{
    // at the beginning start reading from first value
    i = 0;
    printk(KERN_INFO "[PPG] open device driver\n");

    return 0;
}

int PPG_release(struct inode *inode, struct file *filp)
{    
    printk(KERN_INFO "[PPG] release device driver\n");

    return 0;
}

struct file_operations PPG_fops = {
    .owner = THIS_MODULE,
    .read = PPG_read,
    .open = PPG_open,
    .release = PPG_release,
};

static int __init PPG_module_init(void)
{
    printk(KERN_INFO "Loading PPG_module\n");

    alloc_chrdev_region(&PPG_dev, 0, 1, "PPG_dev");
    printk(KERN_INFO "%s\n", format_dev_t(buffer, PPG_dev));

    myclass = class_create(THIS_MODULE, "PPG_sys");
    device_create(myclass, NULL, PPG_dev, NULL, "PPG_dev");

    cdev_init(&PPG_cdev, &PPG_fops);
    PPG_cdev.owner = THIS_MODULE;
    cdev_add(&PPG_cdev, PPG_dev, 1);

    return 0;
}

static void __exit PPG_module_cleanup(void)
{
    printk(KERN_INFO "Cleaning-up PPG_dev.\n");

    device_destroy(myclass, PPG_dev );
    cdev_del(&PPG_cdev);
    class_destroy(myclass);
    unregister_chrdev_region(PPG_dev, 1);
}

module_init(PPG_module_init);
module_exit(PPG_module_cleanup);

MODULE_AUTHOR("Nicola Dilillo");
MODULE_LICENSE("GPL");
