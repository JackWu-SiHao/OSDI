#ifndef _MY_DEVICE_H
#define _MY_DEVICE_H

#include <linux/fs.h>
#include <linux/sched.h>
#include <linux/errno.h>
#include <asm/current.h>
#include <asm/segment.h>
#include <asm/uaccess.h>

char my_data[512];

static int my_open(struct inode *inode,struct file *filep);
static int my_release(struct inode *inode,struct file *filep);
static ssize_t my_read(struct file *filep,char *buff,size_t count,loff_t *offp );
static ssize_t my_write(struct file *filep,const char *buff,size_t count,loff_t *offp );
int my_ioctl(struct inode *inode, struct file *filp, unsigned int cmd, unsigned long arg);

struct file_operations my_fops={
	open: my_open,
	read: my_read,
	write: my_write,
	release:my_release,
	ioctl: my_ioctl,
};

static int my_open(struct inode *inode,struct file *filep)
{
	/* initialize kernel buffer */
	return 0;
}

static int my_release(struct inode *inode,struct file *filep)
{
	/* free memory here */
	return 0;
}

int my_ioctl(struct inode *inode, struct file *filp, unsigned int cmd, unsigned long arg)
 {
    printk("device ioctl\n");
    return 0;
 }

static ssize_t my_read(struct file *filep,char *buff,size_t count,loff_t *offp )
{
	/* function to copy kernel space buffer to user space*/
	if ( copy_to_user(buff,my_data,strlen(my_data)) != 0 )
		printk( "Kernel -> userspace copy failed!\n" );
	return count;

}
static ssize_t my_write(struct file *filep,const char *buff,size_t count,loff_t *offp )
{
	/* function to copy user space buffer to kernel space*/
	if ( copy_from_user(my_data,buff,count) != 0 )
		printk( "Userspace -> kernel copy failed!\n" );
	return count;
}
#endif
