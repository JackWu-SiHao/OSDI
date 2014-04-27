#include <stdio.h>
#include <stdlib.h>

char* buffer0;;
main(){
    int i = 16000000;
    char *buffer1;

    /* Stop, before allocate */
    getchar();
    buffer0 = (char*) malloc(i);
    buffer1 = (char*) malloc(i);

    /* After allocate, but not write */
    getchar();
    int n;

    for(n = 0 ; n<i ; n++){
        buffer1[n] = n%26+'a';
        buffer0[n] = n%26+'a';
    }

    /* After write to the allocated memory */
    getchar();
    return 0;
}
