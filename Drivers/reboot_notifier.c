#include <linux/module.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/reboot.h>
#include <linux/notifier.h>

static int reboot_notify(struct notifier_block *nb, unsigned long action, void *data)
{
	pr_info("This %s is called on system reboot\n", __func__);
	return NOTIFY_DONE;
}

struct notifier_block notify = {
	.notifier_call = reboot_notify,
	.priority = INT_MIN,
};

static int notifier_init(void)
{
	int ret = -1;

	ret = register_reboot_notifier(&notify);
	if (ret) {
		pr_err("Reboot notifier registration failed\n");
		return ret;
	}

	return 0;
}

static void notifier_exit(void)
{
	unregister_reboot_notifier(&notify);
}

module_init(notifier_init);
module_exit(notifier_exit);

MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("Sample module to check working of reboot notifiers");
