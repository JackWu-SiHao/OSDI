#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/kthread.h>
#include <linux/sched.h>

static struct task_struct *thread1;

int thread_function() {
	printk(KERN_INFO "hello\n");
	return 0;
}

int thread_init(void) {
	char thread_name[10] = "my_thread";
	printk(KERN_INFO "thread initialize\n");
	thread1 = kthread_create(thread_function, NULL, thread_name);
	if ( thread1 )
	{
		wake_up_process(thread1);
	}
	return 0;
}

void thread_cleanup(void) {
	printk(KERN_INFO "thread cleanup\n");
}

MODULE_LICENSE("GPL");
module_init(thread_init);
module_exit(thread_cleanup);
