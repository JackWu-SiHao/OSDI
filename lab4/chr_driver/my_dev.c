#include <linux/module.h>
#include <linux/init.h>
#include "my_dev.h"

#define MODULE_NAME "my_dev"
#define MAJOR_NUM	60

MODULE_AUTHOR("Si-Hao Wu");
MODULE_DESCRIPTION("A simple char driver");
MODULE_LICENSE("GPL");

static int my_init(void);
static void my_exit(void);

module_init(my_init);
module_exit(my_exit);

static int my_init(void) {
	printk(KERN_INFO  "/******* Welcome to mydev_0256043 ******/\n");
	if(register_chrdev(MAJOR_NUM, MODULE_NAME, &my_fops))
		printk("<1>%s can't get major %d\n", MODULE_NAME, MAJOR_NUM);
		return (-EBUSY);
	return 0;
}

static void my_exit(void) {
	printk(KERN_INFO   "/****** Bye Bye ******/\n");
	unregister_chrdev(MAJOR_NUM, MODULE_NAME);
	return;
}

