#include<linux/module.h>
#include<linux/kernel.h>
#include<linux/mm.h>
#include<linux/mm_types.h>
#include<linux/file.h>
#include<linux/fs.h>
#include<linux/path.h>
#include<linux/slab.h>
#include<linux/dcache.h>
#include<linux/sched.h>
#include<linux/uaccess.h>
#include<linux/fs_struct.h>
#include <asm/tlbflush.h>
#include<linux/uaccess.h>
#include<linux/device.h>


#define DEVNAME "democdev"

static int major;
atomic_t  device_opened;
static int  count_read;
static struct class *demo_class;
struct device *demo_device;

static char *d_buf = NULL;


static int demo_open(struct inode *inode, struct file *file)
{
        atomic_inc(&device_opened);  //atomically increment the device_opened count
        try_module_get(THIS_MODULE); 
        printk(KERN_INFO "Device opened successfully\n");
        return 0;
}

static int demo_release(struct inode *inode, struct file *file)
{
        atomic_dec(&device_opened);
        module_put(THIS_MODULE);
        printk(KERN_INFO "Device closed successfully\n");
        return 0;
}

static ssize_t demo_read(struct file *filp,
                           char *ubuf,
                           size_t length,
                           loff_t * offset)
{ 
        char d_buf[100] = "mcdivyansh\n";
	// if(length < 4096)
	// 	return -EINVAL;
	if(copy_to_user(ubuf, d_buf, 5))
		return -EINVAL;
        printk(KERN_INFO "In read \n");
        if (count_read>5){
                printk(KERN_INFO "Ho gya bhai iska toh\n");
                return 0;
        } 

        count_read++;
        return 4096;
}

static ssize_t
demo_write(struct file *filp, const char *buff, size_t len, loff_t * off)
{
	    if(copy_from_user(d_buf, buff, len))
		    return -EINVAL;
            printk(KERN_INFO "In write\n");
            return 8;
}

static struct file_operations fops = {
        .read = demo_read,
        .write = demo_write,
        .open = demo_open,
        .release = demo_release,
}; 
// here is the mapping for the system calls and modified implementations

static char *demo_devnode(struct device *dev, umode_t *mode)
{
        if (mode && dev->devt == MKDEV(major, 0))
                *mode = 0666;
        return NULL;
}

int init_module(void)
{
        int err;
	printk(KERN_INFO "Hello kernel\n");
            
        major = register_chrdev(0, DEVNAME, &fops);
        err = major;
        if (err < 0) {      
             printk(KERN_ALERT "Registering char device failed with %d\n", major);   
             goto error_regdev;
        }                 
        
        demo_class = class_create(THIS_MODULE, DEVNAME);
        err = PTR_ERR(demo_class);
        if (IS_ERR(demo_class))
                goto error_class;

        demo_class->devnode = demo_devnode;

        demo_device = device_create(demo_class, NULL,
                                        MKDEV(major, 0),
                                        NULL, DEVNAME);
        err = PTR_ERR(demo_device);
        if (IS_ERR(demo_device))
                goto error_device;
 
        d_buf = kzalloc(4096, GFP_KERNEL);
        printk(KERN_INFO "I was assigned major number %d. To talk to\n", major);                                                              
        atomic_set(&device_opened, 0);
       

	return 0;

error_device:
         class_destroy(demo_class);
error_class:
        unregister_chrdev(major, DEVNAME);
error_regdev:
        return  err;
}

void cleanup_module(void)
{
	kfree(d_buf);
        device_destroy(demo_class, MKDEV(major, 0));
        class_destroy(demo_class);
        unregister_chrdev(major, DEVNAME);
	printk(KERN_INFO "Goodbye kernel\n");
}

MODULE_AUTHOR("deba@cse.iitk.ac.in");
MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("Demo modules");
