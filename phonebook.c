#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/semaphore.h>
#include <linux/uaccess.h>

MODULE_AUTHOR("Krasin Vyacheslav");
MODULE_LICENSE("GPL");
		
int scull_minor = 0;
int scull_major = 0;	

struct char_device {
	char data[100];
} device;

struct cdev *p_cdev;

ssize_t scull_read(struct file *flip, char __user *buf, size_t count,
				loff_t *f_pos)
{
	int rv;

	printk(KERN_INFO "scull: read from device\n");

	rv = copy_to_user(buf, device.data, count);

	return rv;
}

ssize_t scull_write(struct file *flip, const char __user *buf, size_t count,
				loff_t *f_pos)
{
	int rv;

	printk(KERN_INFO "scull: write to device\n");

	rv = copy_from_user(device.data, buf, count);

	return rv;
}

int scull_open(struct inode *inode, struct file *flip)
{
	printk(KERN_INFO "scull: device is opend\n");

	return 0;
}

int scull_release(struct inode *inode, struct file *flip)
{
	printk(KERN_INFO "scull: device is closed\n");

	return 0;
}

struct file_operations scull_fops = {		
	.owner = THIS_MODULE,			
	.read = scull_read,
	.write = scull_write,
	.open = scull_open,
	.release = scull_release,
};

void scull_cleanup_module(void)
{
	dev_t devno = MKDEV(scull_major, scull_minor);

	cdev_del(p_cdev);

	unregister_chrdev_region(devno, 1); 
}

static int scull_init_module(void)
{
	int rv;
	dev_t dev;

	rv = alloc_chrdev_region(&dev, scull_minor, 1, "scull");	

	if (rv) {
		printk(KERN_WARNING "scull: can't get major %d\n", scull_major);
		return rv;
	}

	scull_major = MAJOR(dev);

	p_cdev = cdev_alloc();
	cdev_init(p_cdev, &scull_fops);

	p_cdev->owner = THIS_MODULE;
	p_cdev->ops = &scull_fops;

	rv = cdev_add(p_cdev, dev, 1);

	if (rv)
		printk(KERN_NOTICE "Error %d adding scull", rv);

	printk(KERN_INFO "scull: register device major = %d minor = %d\n", scull_major, scull_minor);

	return 0;
}

module_init(scull_init_module);
module_exit(scull_cleanup_module);
