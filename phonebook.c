#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/semaphore.h>
#include <linux/uaccess.h>

MODULE_AUTHOR("Krasin Vyacheslav");
MODULE_LICENSE("GPL");
		
int phonebook_minor = 0;
int phonebook_major = 0;	

struct char_device {
	char data[100];
} device;

struct cdev *p_cdev;

ssize_t phonebook_read(struct file *flip, char __user *buf, size_t count,
				loff_t *f_pos)
{
	int rv;

	printk(KERN_INFO "phonebook: read from device\n");

	rv = copy_to_user(buf, device.data, count);

	return rv;
}

ssize_t phonebook_write(struct file *flip, const char __user *buf, size_t count,
				loff_t *f_pos)
{
	int rv;

	printk(KERN_INFO "phonebook: write to device\n");

	rv = copy_from_user(device.data, buf, count);

	return rv;
}

int phonebook_open(struct inode *inode, struct file *flip)
{
	printk(KERN_INFO "phonebook: device is opend\n");

	return 0;
}

int phonebook_release(struct inode *inode, struct file *flip)
{
	printk(KERN_INFO "phonebook: device is closed\n");

	return 0;
}

struct file_operations phonebook_fops = {		
	.owner = THIS_MODULE,			
	.read = phonebook_read,
	.write = phonebook_write,
	.open = phonebook_open,
	.release = phonebook_release,
};

void phonebook_cleanup_module(void)
{
	dev_t devno = MKDEV(phonebook_major, phonebook_minor);

	cdev_del(p_cdev);

	unregister_chrdev_region(devno, 1); 
}

static int phonebook_init_module(void)
{
	int rv;
	dev_t dev;

	rv = alloc_chrdev_region(&dev, phonebook_minor, 1, "phonebook");	

	if (rv) {
		printk(KERN_WARNING "phonebook: can't get major %d\n", phonebook_major);
		return rv;
	}

	phonebook_major = MAJOR(dev);

	p_cdev = cdev_alloc();
	cdev_init(p_cdev, &phonebook_fops);

	p_cdev->owner = THIS_MODULE;
	p_cdev->ops = &phonebook_fops;

	rv = cdev_add(p_cdev, dev, 1);

	if (rv)
		printk(KERN_NOTICE "Error %d adding phonebook", rv);

	printk(KERN_INFO "phonebook: register device major = %d minor = %d\n", phonebook_major, phonebook_minor);

	return 0;
}

module_init(phonebook_init_module);
module_exit(phonebook_cleanup_module);
