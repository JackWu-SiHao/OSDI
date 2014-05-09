#  head.s contains the 32-bit startup code.
#  Two L3 task multitasking. The code of tasks are in kernel area, 
#  just like the Linux. The kernel code is located at 0x10000. 

SCRN_SEL	= 0x18

.global startup_32
.text
.globl idt,gdt
startup_32:
	movl $0x10,%eax
	mov %ax,%ds
#	mov %ax,%es
	lss init_stack,%esp

# setup base fields of descriptors.
	call setup_idt
	call setup_gdt
	movl $0x10,%eax		# reload all the segment registers
	mov %ax,%ds		# after changing gdt. 
	mov %ax,%es
	mov %ax,%fs
	mov %ax,%gs
	lss init_stack,%esp

# setup up timer 8253 chip.
	movb $0x36, %al
	movl $0x43, %edx
	outb %al, %dx
	movl $11930, %eax        # timer frequency 100 HZ 
	movl $0x40, %edx
	outb %al, %dx
	movb %ah, %al
	outb %al, %dx

# setup timer & system call interrupt descriptors.
	movl $0x00080000, %eax	
	movw $timer_interrupt, %ax
	movw $0x8E00, %dx
	movl $0x08, %ecx              # The PC default timer int.
	lea idt(,%ecx,8), %esi
	movl %eax,(%esi) 
	movl %edx,4(%esi)
	movw $system_interrupt, %ax
	movw $0xef00, %dx
	movl $0x80, %ecx
	lea idt(,%ecx,8), %esi
	movl %eax,(%esi) 
	movl %edx,4(%esi)

# unmask the timer interrupt.
#	movl $0x21, %edx
#	inb %dx, %al
#	andb $0xfe, %al
#	outb %al, %dx

#Jump to main
	pushl $0	
	pushl $Loop_ret		# return address for main, if it decides to.
	pushl $main
	ret
Loop_ret:
	jmp Loop_ret;


/****************************************/
setup_gdt:
	lgdt lgdt_opcode
	ret

setup_idt:
	lea ignore_int,%edx
	movl $0x00080000,%eax
	movw %dx,%ax		/* selector = 0x0008 = cs */
	movw $0x8E00,%dx	/* interrupt gate - dpl=0, present */
	lea idt,%edi
	mov $256,%ecx
rp_sidt:
	movl %eax,(%edi)
	movl %edx,4(%edi)
	addl $8,%edi
	dec %ecx
	jne rp_sidt
	lidt lidt_opcode
	ret

# -----------------------------------
write_char:
	push %gs
	pushl %ebx
#	pushl %eax
	mov $SCRN_SEL, %ebx
	mov %bx, %gs
	movl scr_loc, %ebx
	shl $1, %ebx
	movb %al, %gs:(%ebx)
	shr $1, %ebx
	incl %ebx
	cmpl $2000, %ebx
	jb 1f
	movl $0, %ebx
1:	movl %ebx, scr_loc	
#	popl %eax
	popl %ebx
	pop %gs
	ret

/***********************************************/
/* This is the default interrupt "handler" :-) */
.align 2
ignore_int:
	push %ds
	pushl %eax
	movl $0x10, %eax
	mov %ax, %ds
#	movl $67, %eax            /* print 'C' */
#	call write_char
die:
	jmp die /*May some exception happended */
	popl %eax
	pop %ds
	iret

/* Timer interrupt handler */ 
.align 2
timer_interrupt:
	push %ds
	push %es
	pushl %eax
	movl $0x10, %eax
	mov %ax, %ds
	mov %ax, %es
	movb $0x20, %al
	outb %al, $0x20
	call timer_handler
	popl %eax
	pop %es
	pop %ds
	iret

/* system call handler */
.align 2
system_interrupt:
	push %ds
	push %es
	pushl %edx
	pushl %ecx
	pushl %ebx
	pushl %eax
	movl $0x10, %edx
	mov %dx, %ds
	mov %dx, %es
	call write_char
	popl %eax
	popl %ebx
	popl %ecx
	popl %edx
	pop %es
	pop %ds
	iret

/*********************************************/
#current:.long 0
scr_loc:.long 0

.align 2
lidt_opcode:
	.word 256*8-1		# idt contains 256 entries
	.long idt		# This will be rewrite by code. 
lgdt_opcode:
	.word (end_gdt-gdt)-1	# so does gdt 
	.long gdt		# This will be rewrite by code.

	.align 8
idt:	.fill 256,8,0		# idt is uninitialized

gdt:	.quad 0x0000000000000000	/* NULL descriptor */
	.quad 0x00c09a00000007ff	/* 8Mb 0x08, base = 0x00000 */
	.quad 0x00c09200000007ff	/* 8Mb 0x10 */
	.quad 0x00c0920b80000002	/* screen 0x18 - for display */
	.fill 252,8,0			/* space for LDT's and TSS's etc */

end_gdt:
	.fill 128,4,0
init_stack:                          # Will be used as user stack for task0.
	.long init_stack
	.word 0x10
