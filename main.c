#include <stdio.h>
#include <stdlib.h>
#include <thread.h>
#include <sem.h>
Sem_T mutex;


int thread_func1(void* args){
    printf("Hello from thread 1, waiting for thread 2!\n");
    int code = Thread_join(*(int*)args);
    printf("joined with code %d!\n",code);
}

int thread_func2(void* args){
    printf("%s from thread 2, returning!\n",args);    

    //critical section
    Sem_wait(&mutex);

    for(int i=0;i<1000;i++){
        printf("%d\n",i);
    }

    Sem_signal(&mutex);
    Thread_exit(32);
}

int main(void){
    Thread_init();
    Sem_init(&mutex, 1);

    printf("hello from main thread!\n");
    int thread2;
    int thread1 = Thread_new(thread_func1,&thread2,sizeof(int));
    thread2 = Thread_new(thread_func2,"hallo00000000000000000000!",sizeof("hallo00000000000000000000!"));
    Thread_join(thread1);
    thread2 = Thread_new(thread_func2,"hallo00000000000000000000!",sizeof("hallo00000000000000000000!"));
    thread2 = Thread_new(thread_func2,"hallo00000000000000000000!",sizeof("hallo00000000000000000000!"));
    thread2 = Thread_new(thread_func2,"hallo00000000000000000000!",sizeof("hallo00000000000000000000!"));
    thread2 = Thread_new(thread_func2,"hallo00000000000000000000!",sizeof("hallo00000000000000000000!"));

    Thread_join(0);
    printf("hello again from main thread!\n");
    Thread_exit(0);
}

