#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/kthread.h>  // for threads
#include <linux/sched.h>  // for task_struct
#include <linux/rcupdate.h>
#include <linux/oom.h>

#define TASKS_PTR_SIZE 1024

static struct task_struct *task_mmkiller;


int thread_function(void *);
int thread_init(void);
void thread_cleanup(void);

int thread_function(void *data) {
	// daemonize("thread_function");
	struct task_struct *tasks_ptr[TASKS_PTR_SIZE];
	struct task_struct *task;
	struct task_struct *p;
	unsigned long max_mm_rss;
	unsigned int readPos = 0;

	printk(KERN_INFO "In Memory Killer\n");
	rcu_read_lock();
	for_each_process(task) {
		p = find_lock_task_mm(task);
		if(!p)
			continue;
		printk(KERN_INFO "Name:%s, PID[%d], RSS:[%lu]\n", task->comm, task->pid, get_mm_rss(task->mm));
		// printk(KERN_INFO "Name:%s, PID[%d]\n", task->comm, task->pid);
		tasks_ptr[readPos++] = task;
		task_unlock(task);
	}

	rcu_read_unlock();
	set_current_state(TASK_INTERRUPTIBLE);
	schedule_timeout(10*HZ);
	return 0;
}

int thread_init(void) {

    char thread_name[50] = "Memory Killer";
    printk(KERN_INFO "in init\n");
    task_mmkiller = kthread_create(&thread_function, NULL, thread_name);
    if((task_mmkiller))
    {
        printk(KERN_INFO "in if\n");
        wake_up_process(task_mmkiller);
    }

    return 0;
}


void thread_cleanup(void) {
	printk(KERN_INFO "Leaving Memory Killer!!\n");
}

MODULE_LICENSE("GPL");
module_init(thread_init);
module_exit(thread_cleanup);
