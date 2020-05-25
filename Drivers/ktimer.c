#include <linux/init.h>
#include <linux/module.h>
#include <linux/workqueue.h>
#include <linux/timer.h>

static struct work_struct timer_work;

static void timer_exp_fn(unsigned long data)
{
	pr_info("The %s was called after timer expiration\n", __func__);
}

static void timer_work_fn(struct work_struct *work)
{
	struct timer_list timer;
	int delay = 5 * HZ;

	/* Initialise the timer */
	init_timer(&timer);

	/* setup delay and funtion to be exected on timer expiration */
	timer.expires = jiffies + delay; /* delay in system ticks */
	timer.function = timer_exp_fn;

	/* activate the timer */
	add_timer(&timer);
}

static int ktimer_init(void)
{
	INIT_WORK(&timer_work, timer_work_fn);

	schedule_work(&timer_work);

	return 0;
}

static void ktimer_exit(void)
{
	cancel_work_sync(&timer_work);
}

module_init(ktimer_init);
module_exit(ktimer_exit);

MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("Sample module to check working of kernel timers");
