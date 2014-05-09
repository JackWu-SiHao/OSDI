#include <linux/kernel.h>
#include <linux/linkage.h>

extern buffer_inuse_length;

asmlinkage int sys_myclean(void) {
	buffer_inuse_length = 0;
	return 0;
}
