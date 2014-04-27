#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/kthread.h>
#include <linux/sched.h>
#include <linux/string.h>
#include <linux/mm.h>
#include <linux/highmem.h>
#include <linux/hardirq.h>
#include <asm/page_types.h>  /* for PAGE_SIZE */
#include <asm/pgtable.h>

static struct task_struct *task_dump_page_usage;

int dump_page_usage(void *);
int thread_init(void);
void thread_cleanup(void);

int dump_page_usage(void *data) {

    struct task_struct *g, *p;
    struct mm_struct *mm;
    struct vm_area_struct *vma;
    unsigned long address;
    unsigned int page_count = 0;
    spinlock_t *ptl;
    pgd_t *pgd;
    pud_t *pud;
    pmd_t *pmd;
    pte_t *pte;

    do_each_thread(g, p) {
        task_lock(p);
        if ( strcmp(p->comm, "reclim-me") == 0 ) {
            mm = p->mm;
            if ( mm ) {
                printk(KERN_INFO "[PID]:%-8d | [Name]:%-8s | [RSS]:%-8lu\n",
                    p->pid, p->comm, get_mm_rss(mm));

                /* for each VMA */
                for(vma = mm->mmap ; (vma = vma->vm_next) != NULL;) {
                    address = vma->vm_start;

                    /* _PAGE_PRESENT Page is resident in memory and not swapped out */
                    pgd = pgd_offset(mm, address);
                    if(!pgd_present(*pgd))
                        continue;

                    pud = pud_offset(pgd, address);
                    if(!pud_present(*pud))
                        continue;

                    pmd = pmd_offset(pud, address);
                    if(!pmd_present(*pmd))
                        continue;

                    /* for each Page Table entry */
                    pte = pte_offset_map_lock(mm, pmd, address, &ptl);
                    for(; address < vma->vm_end; pte++, address += PAGE_SIZE) {
                        if(!pte_present(*pte))
                            continue;
                        page_count++;
                    }
                    pte_unmap_unlock(pte - 1, ptl);
                }
            }
            break;
        }
        task_unlock(p);
    } while_each_thread(g, p);

    printk(KERN_INFO "[PAGE USAGE]:%-8u\n", page_count);

    return 0;
}

int thread_init(void) {

    char thread_name[50] = "Dump VM Usage";
    task_dump_page_usage = kthread_create(&dump_page_usage, NULL, thread_name);
    if((task_dump_page_usage))
    {
        printk(KERN_INFO "Enter Dump Page Usage Kernel Module\n");
        wake_up_process(task_dump_page_usage);
    }

    return 0;
}

void thread_cleanup(void) {
    printk(KERN_INFO "Leaving Dump Page Usage!\n");
}

MODULE_LICENSE("GPL");
module_init(thread_init);
module_exit(thread_cleanup);
