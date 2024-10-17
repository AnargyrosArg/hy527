#include <stdio.h>
#include <stdlib.h>
#include <thread.h>


extern void _thrstart(void);

int thread_func(void* args){
    printf("Hello from another thread! got i=%d\n",*(int*)args);
}

int main(void){    
    Thread_init();
    int i=5;
    printf("hello from main thread!\n");
    Thread_new(thread_func,&i,sizeof(i));
    Thread_pause();

    printf("hello again from main thread!\n");
}