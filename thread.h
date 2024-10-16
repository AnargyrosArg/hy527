#ifndef THREAD_INCLUDED
#define THREAD_INCLUDED

#include <stddef.h>
#include <queue.h>


#define MAX_N_THREADS 1000

typedef struct {
    void* sp; //stack pointer for thread's stack
    int id;
} thread_t;



extern void Thread_init(void);
extern int Thread_new(int func(void *), void *args, size_t nbytes, ...);
extern void Thread_exit(int code);
extern int Thread_self(void);
extern int Thread_join(int tid);
extern void Thread_pause(void);

#endif