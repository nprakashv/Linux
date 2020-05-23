#include <linux/init.h>
#include <linux/module.h>
#include <linux/fs.h>
#include <linux/kobject.h>
#include <linux/sysfs.h>
#include <linux/errno.h>
#include <linux/workqueue.h>
#include <linux/types.h>
#include <linux/gfp.h>
#include <linux/mm.h>

#define FNAME_MAX_LEN	1024

/* Function to get the filename from userspace */
ssize_t fname_store(struct kobject *kobj, struct kobj_attribute *attr, const char *buf, size_t count);

/* Function to read the file contents */
static void fread_work(struct work_struct *work);

char filename[FNAME_MAX_LEN];

static struct kobject *kobj = NULL;
static struct work_struct read_work;
static struct kobj_attribute kobjattr = __ATTR(fname, 0660, NULL, fname_store);

static void fread_work(struct work_struct *work)
{
	struct file *filp;
	struct kstat stat;
	int ret;
	loff_t filesize = 0;
	unsigned int order;
	struct page *page =  NULL;
	void *addr = NULL;

	filp = filp_open(filename, O_RDONLY, 0);

	if (!filp) {
		pr_err("File open failed. Bad file descriptor\n");
		return;
	}

	/* get the size of the file */
	ret = vfs_getattr(&filp->f_path, &stat);
	filesize = stat.size;

	/* get the order of pages to be allocated */
	order = get_order(filesize);

	/* allocate pages */
	page = alloc_pages(GFP_KERNEL, order);
	if (!page) {
		pr_err("Not enough memory\n");
		filp_close(filp, NULL);
		return;
	}

	/* get the virtual address of the page allocated */
	addr = page_address(page);

	if (addr) {
		memset(addr, 0, filesize);

		/* Read file contents starting from offset 0 */
		ret = kernel_read(filp, 0, addr, filesize);
		if (ret != filesize) {
			pr_err("File read failed\n");
			free_pages((unsigned long)addr, order);
			filp_close(filp, NULL);
			return;
		}
		free_pages((unsigned long)addr, order);
	}

	filp_close(filp, NULL);
}

ssize_t fname_store(struct kobject *kobj, struct kobj_attribute *attr, const char *buf, size_t count)
{
	strlcpy(filename, buf, count);
	schedule_work(&read_work);
	return count;
}

static int mod_init(void)
{
	int ret = -1;

	kobj = kobject_create_and_add("fread", kernel_kobj);
	if(!kobj)
		return -ENOMEM;

	ret = sysfs_create_file(kobj, &kobjattr.attr);
	if(!ret) {
		pr_err("Failed to create sysfs file\n");
		kobject_put(kobj);
		return -ENOMEM;
	}

	INIT_WORK(&read_work, fread_work);

	return 0;
}

static void mod_exit(void)
{
	sysfs_remove_file(kobj, &kobjattr.attr);
	kobject_put(kobj);
}

module_init(mod_init);
module_exit(mod_exit);

MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("Sample driver to read file contents from kernel space");
