#include <linux/unistd.h>
#include <sys/syscall.h>
#include <stdio.h>

#define __NR_myread		338
#define __NR_mywrite	339
#define __NR_myclean	340

int main(void) {
	char message[20] = "0256043 hello lab4";
	char receive_mes[20];
	syscall(__NR_myclean);
	syscall(__NR_mywrite, message, 20);
	syscall(__NR_myread, receive_mes, 20);
	printf("%s\n", receive_mes);
	syscall(__NR_myclean);
	return 0;
}
