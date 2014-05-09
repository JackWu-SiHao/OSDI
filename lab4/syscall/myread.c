#include <linux/kernel.h>
#include <linux/linkage.h>
#include <asm/uaccess.h>

#define BUFFER_SIZE 4096
int buffer_inuse_length = 0;
char buffer[BUFFER_SIZE];

asmlinkage void sys_myread(char *array, int bytes) {
	/* check whether buffer has enogh data to read */
	if ( buffer_inuse_length >= bytes)
	{
		copy_to_user(array, buffer, bytes);
		buffer_inuse_length -= bytes;
		int i;
		/* move remaining data to base */
		for (i = 0; i < buffer_inuse_length; ++i)
		{
			buffer[i] = buffer[i+bytes];
		}
	}
	else {
		/* read bytes over buffer, error */
	}
}
