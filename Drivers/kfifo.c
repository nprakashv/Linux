#include <linux/module.h>
#include <linux/init.h>
#include <linux/kfifo.h>
#include <linux/kernel.h>
#include <linux/gfp.h>
#include <linux/errno.h>
#include <linux/string.h>
#include <linux/slab.h>

static struct kfifo kfifo;

static int myinit(void)
{
	int ret = -1;
	char *ptr = "Hello Nikhil Prakash V";
	char *buf = NULL;

	ret = kfifo_alloc(&kfifo, 1000, GFP_KERNEL);
	if (ret) {
		pr_err("Fifo allocation failed. Memory not available\n");
		return -ENOMEM;
	}

	ret = kfifo_in(&kfifo, ptr, strlen(ptr));
	if (!ret) {
		pr_err("FIFO enqueue failed\n");
		goto err_enq;
	}

	buf = (char *)kmalloc(strlen(ptr) + 1, GFP_KERNEL);

	while(!kfifo_is_empty(&kfifo)) {
		kfifo_out(&kfifo, buf, strlen(ptr));
		buf[strlen(ptr)] = '\0';
		pr_info("Buffer is %s\n", buf);
	}

	kfree(buf);
	return 0;

err_enq:
	kfifo_free(&kfifo);
	return -ENOSPC;
}

static void myexit(void)
{
	kfifo_free(&kfifo);
}

module_init(myinit);
module_exit(myexit);

MODULE_DESCRIPTION("Sample driver to check the working of kernel fifo's");
MODULE_LICENSE("GPL");
