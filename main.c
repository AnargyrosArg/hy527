#include <stdio.h>
#include <stdlib.h>
#include <thread.h>


extern void _thrstart(void);

int thread_func1(void* args){
    printf("Hello from thread 1, waiting for thread 2!\n");
    Thread_join(*(int*)args);
    printf("joined!\n");
}

int thread_func2(void* args){
    printf("Hello from thread 2, returning!\n");
}



int main(void){    
    Thread_init();
    printf("hello from main thread!\n");
    int thread2;
    int thread1 = Thread_new(thread_func1,&thread2,sizeof(int));
    thread2 = Thread_new(thread_func2,NULL,0);

    Thread_join(0);
    printf("hello again from main thread!\n");
    Thread_exit(0);
}