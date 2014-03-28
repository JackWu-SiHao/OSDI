#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

int main()
{
	int fd=0,ret=0;
	char buff[512]="Lab4 0256043";

	fd=open("/dev/my_dev",O_RDONLY);

	printf("fd :%d\n",fd);

	write(fd, buff, 80);
	ret=read(fd,buff,80);

	printf("buff: %s\n",buff);
	close(fd);
}


