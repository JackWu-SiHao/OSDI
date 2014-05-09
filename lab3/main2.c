/* This only use for NCTU CS OSDI course */

#include "main.h"
#define NR_TASKS	4
#define NULL		0

struct task_struct tasks[NR_TASKS];

int current = 0;

extern desc_table idt, gdt;

void print_string(char* str, int len)
{
	int i;
	if( len < 0 )
		return;
	for( i = 0; i < len; i++ )
	{
		__asm__ ( "movb 0(%0,%1,1),%%al\n\t"
			"int $0x80\n\t"
			:
			: "b"(str),"c"(i)
		);
	}
}

void timer_handler()
{
	/* TODO: Lab3.3 Write a simple FIFO scheduler at here */
	char z = 'z';
	print_string(&z, 1);
	switch_to(1);
	//move_to_user_mode();

}

int create_task(func task, int id)
{
	/* TODO: Lab3.2 Write the create task program here */
	struct task_struct *p;
	if( id >= NR_TASKS )
		return -1;
	p = &tasks[id];

	/* Fill the task structure here */
	p->uid = id;
	p->state = 2;

	//
	p->tss.eflags = 0x200;

	p->tss.cs = 0x17;
	p->tss.ss = 0;
	p->tss.eip = task;
	p->tss.trace_bitmap = 0x80000000;
	p->ldt[0].a = 0;
	p->ldt[0].b = 0;
	p->ldt[1].a = 0x00003fff;
	p->ldt[1].b = 0x0040fa00;
	p->ldt[2].a = 0x00003fff;
	p->ldt[2].b = 0x0040f200;


	p->tss.eax = 0;
	p->tss.ecx = 0x08;
	p->tss.edx = 0;
	p->tss.ebx = 0x04;

	p->tss.ebp = 0;
	p->tss.esi = 0;
	p->tss.edi = 0;
	p->tss.es = 0x17;
	p->tss.ds = 0x0f;
	p->tss.fs = 0;
	p->tss.gs = 0;
	p->tss.esp0 = &(p->krn_stack[USR_STACK_SIZE-4]);//USR_STACK_SIZE + (long) p;;
	p->tss.ss0 = 0x10;
	p->tss.back_link = 0;


	/* set task first prarmeter */
	p->usr_stack[USR_STACK_SIZE-4] = (char)(id);

	//
	p->tss.esp = &(p->usr_stack[USR_STACK_SIZE-8]);
	set_tss_desc(gdt+(id<<1)+FIRST_TSS_ENTRY,&(p->tss));
	set_ldt_desc(gdt+(id<<1)+FIRST_LDT_ENTRY,&(p->ldt));

	p->state = 0;

	return id;
}

void task_proc(int a)
{
	char c = 'a' + a;

	while(1)
	{
		print_string(&c, 1);
		int i;
		for(i = 0;i < 100000;i++);
	}
}

char str[6];// = "Hello\n";
void main()
{
	str[0] = 'H';str[1] = 'e';str[2] = 'l';str[3] = 'l';str[4] = 'o';str[5] = ' ';

	print_string(str, 6);

	//Create tasks
	create_task(task_proc, 0);
	create_task(task_proc, 1);
	create_task(task_proc, 2);
	create_task(task_proc, 3);

	//Load first task register and LDT descriptor
	__asm__("pushfl ; andl $0xffffbfff,(%esp) ; popfl"); // clear NT flag
	ltr(0);
	lldt(0);

	sti(); //enable interrupt
	//switch_to(0);
	move_to_user_mode();

	for(;;){}
}
