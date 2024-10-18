#include <stdio.h>
#include <stdlib.h>
#include <thread.h>

// int thread_func1(void* args){
//     printf("Hello from thread 1, waiting for thread 2!\n");
//     int code = Thread_join(*(int*)args);
//     printf("joined with code %d!\n",code);
// }

// int thread_func2(void* args){
//     printf("Hello from thread 2, returning!\n");
//     Thread_exit(32);
// }

// int main(void){
//     Thread_init();
//     printf("hello from main thread!\n");
//     int thread2;
//     int thread1 = Thread_new(thread_func1,&thread2,sizeof(int));
//     thread2 = Thread_new(thread_func2,NULL,0);

//     Thread_join(0);
//     printf("hello again from main thread!\n");
//     Thread_exit(0);
// }

int thread_func(void *args)
{
    printf("Hello from thread ,exiting!\n");
}

int main(void)
{
    Thread_init();
    for (int i = 0; i < 100000; i++)
    {
        Thread_new(thread_func,NULL,0);
    }
    Thread_join(0);
    printf("main done!\n");
}