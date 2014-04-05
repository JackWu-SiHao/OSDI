#include <linux/unistd.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/kthread.h>  // for threads
#include <linux/sched.h>  // for task_struct
#include <linux/mm.h>
#include <linux/string.h>

#define TASKS_PTR_SIZE 4096

asmlinkage long (*sys_tkill)(int pid, int sig);

static struct task_struct *task_mmkiller;


int thread_function(void *);
int thread_init(void);
void thread_cleanup(void);

int thread_function(void *data) {

	unsigned int readPos = 0, min_pos, i, j;
	unsigned long min_rss, tmp_rss;
	struct task_struct *g, *p;
	struct task_struct *tasks_ptr[TASKS_PTR_SIZE];

	unsigned long *sys_call_table = (unsigned long*)(0xc07992b0);
	sys_tkill = (sys_call_table[__NR_tkill]);

	do_each_thread(g, p) {
		struct mm_struct *mm;
		if (!thread_group_leader(p))
			continue;

		task_lock(p);
		mm = p->mm;
		if (mm && (p->real_parent->pid != 2) && (strcmp(p->comm, "Xorg") != 0) ) {
			/*
			 * add only has mm_struct, not kernel thread and not Xorg
			 */
			tasks_ptr[readPos++] = p;
			printk(KERN_INFO
				"(Origin) PID:[%-5d]| Name:%-20s| VM:%-8lu| RSS:%-8lu| OOM_adj: %-3d\n",
				p->pid, p->comm, mm->total_vm, get_mm_rss(mm), p->signal->oom_adj);
		}

		task_unlock(p);
	} while_each_thread(g, p);

	for (i = 0; i < readPos; ++i)
	{
		min_rss = get_mm_rss(tasks_ptr[i]->mm);
		p = tasks_ptr[i];
		min_pos = i;
		for ( j = i+1; j < readPos; ++j )
		{
			tmp_rss = get_mm_rss(tasks_ptr[j]->mm);
			if ( tmp_rss < min_rss )
			{
				min_rss = tmp_rss;
				min_pos = j;
			}
		}
		p = tasks_ptr[i];
		tasks_ptr[i] = tasks_ptr[min_pos];
		tasks_ptr[min_pos] = p;
	}

	for (i = 0; i < readPos; ++i)
	{
		printk(KERN_INFO
			"(Sorted) PID:[%-5d]| Name:%-20s| VM:%-8lu| RSS:%-8lu| OOM_adj: %-3d\n",
			tasks_ptr[i]->pid, tasks_ptr[i]->comm, tasks_ptr[i]->mm->total_vm,
			get_mm_rss(tasks_ptr[i]->mm), tasks_ptr[i]->signal->oom_adj);
	}

	printk(KERN_INFO "Kill PID[%-5d], Name:%-20s, RSS:%-8lu",
		tasks_ptr[readPos-1]->pid, tasks_ptr[readPos-1]->comm,
		get_mm_rss(tasks_ptr[readPos-1]->mm));
	sys_tkill(tasks_ptr[readPos-1]->pid, SIGKILL);
	// sys_tkill(6704, SIGKILL);

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
