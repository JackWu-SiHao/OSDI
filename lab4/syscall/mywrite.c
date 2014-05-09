#include <linux/kernel.h>
#include <linux/linkage.h>
#include <asm/uaccess.h>

#define BUFFER_SIZE 4096
extern int buffer_inuse_length;
extern char buffer[BUFFER_SIZE];

asmlinkage void sys_mywrite(char *array, int bytes) {
	size_t buffer_remaining_size = BUFFER_SIZE - buffer_inuse_length;
	/* allow to write */
	if ( buffer_remaining_size > bytes ) {
		copy_from_user(buffer+buffer_inuse_length, array, bytes);
		buffer_inuse_length += bytes;
	}
	else {
		/* not enogh buffer, error handling */
	}
}
