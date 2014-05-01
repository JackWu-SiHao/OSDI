#include <unistd.h> /* system call */
#include <sys/ioctl.h>
#include <fnctl.h>
#include <stdio.h>


#define MAJOR_NUM 100
#define IOCTL_HELLO_RAMDISK _IO(MAJOR_NUM, 1)


void ioctl_hello_ramdisk(int fd);
void ioctl_get_msg(int fd);
void ioctl_set_msg(int fd, char *message);


void ioctl_hello_ramdisk(int fd)
{
    int ret;

    ret = ioctl(fd, )
}


int main(int argc, char const *argv[])
{
    int fd;
    fd = open("/dev/ram0", O_RDWR);
    ioctl_hello_ramdisk(fd);

    return 0;
}
