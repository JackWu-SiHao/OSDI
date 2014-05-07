#include <unistd.h> /* system call */
#include <fcntl.h>  /* O_RDWR */
#include <stdlib.h> /* exit(-1) */
#include <sys/ioctl.h>
#include <stdio.h>


#define DO_ROLLBACK 2
#define MAJOR_NUM 101
#define IOCTL_HELLO_RAMDISK _IO(MAJOR_NUM, 1)


void ioctl_hello_ramdisk(int fd);
void ioctl_get_msg(int fd);
void ioctl_set_msg(int fd, char *message);


void ioctl_hello_ramdisk(int fd)
{
    int ret;
    ret = ioctl(fd, IOCTL_HELLO_RAMDISK, DO_ROLLBACK);

}


int main(int argc, char const *argv[])
{
    int fd;

    fd = open("/dev/ram0", O_RDWR);
    ioctl_hello_ramdisk(fd);

    close(fd);
    return 0;
}
