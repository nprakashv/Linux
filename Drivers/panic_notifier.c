#include <linux/module.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/notifier.h>

static int panic_notify(struct notifier_block *nb, unsigned long action, void *data)
{
	pr_info("This %s is called on system crash scenario\n", __func__);
	return NOTIFY_DONE;
}

struct notifier_block notify = {
	.notifier_call = panic_notify,
	.priority = INT_MIN,
};

static int notifier_init(void)
{
	int ret = -1;

	ret = atomic_notifier_chain_register(&panic_notifier_list, &notify);
	if (ret) {
		pr_err("Reboot notifier registration failed\n");
		return ret;
	}

	return 0;
}

static void notifier_exit(void)
{
	atomic_notifier_chain_unregister(&panic_notifier_list, &notify);
}

module_init(notifier_init);
module_exit(notifier_exit);

MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("Sample module to check working of panic notifiers");
