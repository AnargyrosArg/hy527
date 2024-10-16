#include <stdio.h>
#include <stdlib.h>
#include <thread.h>


extern void _thrstart(void);

int thread_func(void* args){
    printf("Hello from another thread!\n");
    Thread_exit(0);
}

int main(void){    
    Thread_init();
    printf("hello from main thread!\n");
    Thread_new(thread_func,NULL,0);
    Thread_pause();

    printf("hello again from main thread!\n");
}