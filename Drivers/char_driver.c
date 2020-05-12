#include <linux/init.h>
#include <linux/module.h>
#include <linux/sched.h>
#include <linux/types.h>
#include <linux/fs.h>
#include <linux/slab.h>
#include <linux/cdev.h>
#include <asm/uaccess.h>
#include <linux/kernel.h>
#include <linux/notifier.h>
#include <linux/reboot.h>

#ifdef STATIC_MAJOR_ALLOCATION 
#define MAJOR_NUM	222
#define MINOR_NUM	0
#endif

#define MAX_MINOR_NUM	3
#define KB		1024


struct chrdev_struct {
	void *data;
	struct cdev cdev;
	int count;
};

static struct chrdev_struct pvt[MAX_MINOR_NUM];

static dev_t devicenum;

static int chrdev_open(struct inode *i, struct file *filp)
{
	struct chrdev_struct *p = NULL;

	p = container_of(i->i_cdev, struct private_struct, cdev);
	filp->private_data = p;

	return 0;
}

static int chrdev_release(struct inode *i, struct file *filp)
{
	return 0;
}

static ssize_t chrdev_read(struct file *filp, char __user *buf, size_t count, loff_t *fpos)
{
	struct chrdev_struct *p = NULL;

	p = filp->private_data;

	if (count >= p->count) {
		if (*fpos == 0) {
			copy_to_user(buf, p->data, p->count);
			*fpos = *fpos + p->count;
			return p->count;
		} else
			return 0;
	} else if (count < p->count) {
		if (*fpos == 0) {
			copy_to_user(buf, p->data, count);
			*fpos = *fpos + count;
			return count;
		} else
			return 0;
	} else {
		pr_info("Read failed\n");
		return -1;
	}
}

static ssize_t chrdev_write(struct file *filp, const char __user *buf, size_t count, loff_t *fpos)
{
	struct chrdev_struct *p;

	p = filp->private_data;
	*fpos = p->count;

	if (!copy_from_user(p->data + *fpos, buf, count)) {
		p->count = p->count + count;
		return count;
	} else {
		pr_info("Write failed\n");
		return -1;
	}
}

struct file_operations chrdevops = {
	.owner = THIS_MODULE,
	.open = chrdev_open,
	.release = chrdev_release,
	.read = chrdev_read,
	.write = chrdev_write,
};

static int register_character_device(struct chrdev_struct *ptr, int i)
{
	int ret = -1;

	cdev_init(&ptr->cdev, &chrdevops);
	ptr->cdev.owner = THIS_MODULE;
	ret = cdev_add(&ptr->cdev, MKDEV(MAJOR(devicenum), i), 1);
	return ret;
}

static int chardev_init(void)
{
	int ret = -1, i = 0;

#ifdef STATIC_MAJOR_ALLOCATION
	devicenum = MKDEV(MAJOR_NUM, MINOR_NUM);
	ret = register_chrdev_region(devicenum, MAX_MINOR_NUM, "chrdev");
#else
	ret = alloc_chrdev_region(&devicenum, 0, MAX_MINOR_NUM, "chrdev");
#endif
	if (!ret) {
		pr_info("Device number registration success with device"
			"number:%d, majornum:%d, minornum:%d\n",
			devicenum, MAJOR(devicenum), MINOR(devicenum));
	} else {
		pr_info("Device number registration failed with err:%d\n", ret);
		return ret;
	}

	for (i = 0; i < MAX_MINOR_NUM; i++) {
		pvt[i].data = kzalloc(4 * KB, GFP_KERNEL);
		ret = register_character_device(&pvt[i], i);
		if (ret) {
			pr_info("Character device registration failed with err:%d\n", ret);
			goto char_dev_reg_fail;
		}
	}

	pr_info("Character device registration success\n");

	return 0;

char_dev_reg_fail:
	for (i = 0; i < MAX_MINOR_NUM; i++) {
		kfree(pvt[i].data);
	}
	unregister_chrdev_region(devicenum, MAX_MINOR_NUM);
	return ret;
}

static void chardev_exit(void)
{
	unregister_chrdev_region(devicenum, MAX_MINOR_NUM);
	return;
}

module_init(chardev_init);
module_exit(chardev_exit);

MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("Sample character driver");
