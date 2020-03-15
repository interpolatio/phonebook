#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/semaphore.h>
#include <linux/uaccess.h>
#include <linux/string.h>


MODULE_AUTHOR("Krasin Vyacheslav");
MODULE_LICENSE("GPL");
		
int phonebook_minor = 0;
int phonebook_major = 0;	

struct scull_device {
	char data[100];
	size_t size;
} ;
char data_buf[100];
struct user_type {
	char first_name[100];
	char last_name[100];
	//unsigned age;
	//char number[10];
	//char email[100];
};

struct user_base_type{
	struct user_type *user;
	size_t num_user;
} ;
struct user_base_type user_base;

enum Command_type {
	CREATE,
	FIND,
	DELETE,
	NOTH
};
struct cdev *p_cdev;

ssize_t phonebook_read(struct file *flip, char __user *buf, size_t count,
				loff_t *f_pos)
{
	int rv;

	printk(KERN_INFO "phonebook: read from device\n");

	rv = copy_to_user(buf,data_buf, count);	
	
	printk(KERN_INFO "phonebook str: %s\n" , data_buf);

	return rv;
}

ssize_t phonebook_write(struct file *flip, const char __user *buf, size_t count,
				loff_t *f_pos)
{
	int rv;
	char  *token ;
	char *tmp;
	struct user_base_type tmp_user_base;	
	struct user_type *tmp_user = vmalloc(sizeof(struct user_type));
	
	enum Command_type command =NOTH ;
	//struct scull_device *data = (struct scull_device *) flip->private_data;
	ssize_t len = count;//min(data->size - *f_pos,count);

	printk(KERN_INFO "phonebook: write to device\n");
	
	
	rv =  copy_from_user(data_buf, buf, count);
	printk(KERN_INFO "phonebook read: %i\n" , (int)rv);
	printk(KERN_INFO "phonebook str: %s" , data_buf);
	printk(KERN_INFO "phonebook: +++++");
	tmp = vmalloc(count-1);
	memcpy(tmp, data_buf, count-1);
	command = NOTH;
	do {
		//printk(KERN_INFO "phonebook: -----");
		
		token = strsep(&tmp, " ");
		//printk(KERN_INFO "phonebook token: %s\n", token);
		if (token){
			if (command == CREATE){
				if (tmp_user->first_name[0] == '\0')
					strcpy(tmp_user->first_name, token);
				else if (tmp_user->last_name[0] == '\0'){
					strcpy(tmp_user->last_name, token);	
			
					user_base.num_user = user_base.num_user + 1;	
					user_base.user = vmalloc(user_base.num_user);
					
					memcpy(&(user_base.user[0]), tmp_user, sizeof(struct user_base_type));					
					printk(KERN_INFO "phonebook user_base work: %s\n", (user_base.user[0].first_name));
					
				}
			}			
							
			if (strcmp(token, "create") == 0)
				command = CREATE;
				
				
				
				
			//printk(KERN_INFO "phonebook compare: %i\n", strcmp(token, "create"));
			
		}
	}while (token);
	printk(KERN_INFO "phonebook num_user: %i\n" , (int)user_base.num_user);
	printk(KERN_INFO "phonebook user_base: %p\n", user_base.user);
	
	
	vfree(tmp);
	//printk(KERN_INFO "phonebook size user_base->num_user: %li\n", user_base->num_user);
	//*f_pos += len;   
	return len;
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
