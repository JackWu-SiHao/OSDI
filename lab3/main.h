/* This only for NCTU CS OSDI course*/
// TODO: move to task1
#define move_to_user_mode() \
__asm__ ("pushl $0x17\n\t" \
	"pushl %%ecx\n\t" \
	"pushfl\n\t" \
	"pushl $0x0f\n\t" \
	"pushl %%ebx\n\t" \
	"iret\n" \
	:: "b"((tasks[0].tss.eip)), "c"(tasks[0].tss.esp) \
)


/*
 * Entry into gdt where to find first TSS. 0-nul, 1-cs, 2-ds, 3-syscall
 * 4-TSS0, 5-LDT0, 6-TSS1 etc ...
 */
#define FIRST_TSS_ENTRY 4
#define FIRST_LDT_ENTRY (FIRST_TSS_ENTRY+1)
#define _TSS(n) ((((unsigned long) n)<<4)+(FIRST_TSS_ENTRY<<3))
#define _LDT(n) ((((unsigned long) n)<<4)+(FIRST_LDT_ENTRY<<3))

//Load task register
#define ltr(n) __asm__("ltr %%ax"::"a" (_TSS(n)))
//Load LDT
#define lldt(n) __asm__("lldt %%ax"::"a" (_LDT(n)))

#define sti() __asm__ ("sti"::)
#define cli() __asm__ ("cli"::)

//Store task register
#define str(n) \
__asm__("str %%ax\n\t" \
	"subl %2,%%eax\n\t" \
	"shrl $4,%%eax" \

#define _set_tssldt_desc(n,addr,type) \
__asm__ ("movw $104,%1\n\t" \
	"movw %%ax,%2\n\t" \
	"rorl $16,%%eax\n\t" \
	"movb %%al,%3\n\t" \
	"movb $" type ",%4\n\t" \
	"movb $0x00,%5\n\t" \
	"movb %%ah,%6\n\t" \
	"rorl $16,%%eax" \
	::"a" (addr), "m" (*(n)), "m" (*(n+2)), "m" (*(n+4)), \
	 "m" (*(n+5)), "m" (*(n+6)), "m" (*(n+7)) \
	)

#define set_tss_desc(n,addr) _set_tssldt_desc(((char *) (n)),((int)(addr)),"0xe9")
#define set_ldt_desc(n,addr) _set_tssldt_desc(((char *) (n)),((int)(addr)),"0xe2")

#define KRN_STACK_SIZE	1024
#define USR_STACK_SIZE	512


/* Context switch: TA modified version :p */
#define switch_to(n) { \
struct {long a,b;} __tmp; \
__asm__("cmpl %%ecx,current\n\t" \
	"je 1f\n\t" \
	"movw %%dx,%1\n\t" \
	"mov %%ecx, current\n\t" \
	"ljmp *%0\n\t" \
	"clts\n" \
	"1:" \
	::"m" (*&__tmp.a),"m" (*&__tmp.b), \
	"d"(_TSS(n)),"c"(tasks[n].uid)); \
}

typedef void (*func)(int);

typedef struct desc_struct {
	unsigned long a,b; //a is store base[0:15] and limit[0:15], b is store descriptor,DPL,...
} desc_table[256];

struct tss_struct {
	long	back_link;	/* 16 high bits zero */
	long	esp0;
	long	ss0;		/* 16 high bits zero */
	long	esp1;
	long	ss1;		/* 16 high bits zero */
	long	esp2;
	long	ss2;		/* 16 high bits zero */
	long	cr3;
	long	eip;
	long	eflags;
	long	eax,ecx,edx,ebx;
	long	esp;	/* user */
	long	ebp;
	long	esi;
	long	edi;
	long	es;		/* 16 high bits zero */
	long	cs;		/* 16 high bits zero */
	long	ss;		/* 16 high bits zero user*/
	long	ds;		/* 16 high bits zero */
	long	fs;		/* 16 high bits zero */
	long	gs;		/* 16 high bits zero */
	long	ldt;		/* 16 high bits zero */
	long	trace_bitmap;	/* bits: trace 0, bitmap 16-31 */
};

struct task_struct {
	long uid;
	long state;
	/* ldt for this task 0 - zero 1 - cs 2 - ds&ss */
	struct desc_struct ldt[3];
	/* tss for this task */
	struct tss_struct tss;
	char krn_stack[KRN_STACK_SIZE];
	char usr_stack[USR_STACK_SIZE];
};
