#include <thread.h>
#include <assert.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>

static inline _Bool is_aligned(const void *restrict pointer, size_t byte_count)
{ return (uintptr_t)pointer % byte_count == 0; }



extern void _swtch(void* from, void* to);
extern void _thrstart(void);
void cleanup_threads();
extern void _ENDMONITOR(void);

static void* current_thread = NULL; //keeps current running thread
static int nthreads = -1; //number of running threads

// static thread_t thread_pool[MAX_N_THREADS]; //keeps addresses of thread_t objects, index in this array is thread ID

Queue ready_queue;
Queue free_queue;
thread_t main_thread;

void _STARTMONITOR(void){};

void switch_thread(void){
    thread_t* next_thread = peek(&ready_queue);
    dequeue(&ready_queue);
    void* previous_thread = current_thread;
    current_thread = next_thread;
    _swtch(previous_thread,next_thread);
}


void Thread_init(void){
    if(nthreads != -1 || current_thread != NULL){
        fprintf(stderr,"Thread_init on (probably) already initialized state!\n");
    }
    //init ready queue
    initializeQueue(&ready_queue);
    initializeQueue(&free_queue);
    main_thread.id = (int)&main_thread;
    current_thread = &main_thread;
    nthreads = 1;
    // enqueue(&ready_queue,&main_thread);
    //TODO HERE -> pre-emption:signals, interrupts
}


void Thread_pause(void){
    assert(current_thread!=NULL);
    enqueue(&ready_queue,current_thread);
    switch_thread();
}

int Thread_new(int func(void *), void *args, size_t nbytes, ...){
    assert(func!=NULL);
    assert(nbytes >= 0);
    if(args==NULL){
        assert(nbytes==0);
    }else{
        assert(nbytes>0);
    }
    thread_t* new_thread;
    cleanup_threads();
    new_thread = malloc(1024*16 + nbytes + sizeof(thread_t) + 15); //allocate bytes for stack + thread_t type -> +15 to be able to round up to already allocated memory
    assert(new_thread!=NULL);

    void *alligned_addr = (void*)(((uintptr_t)new_thread+15) & ~ (uintptr_t)0x0F); //fixes ptr to be 16 alligned
    void* thread_stack = ((char*)alligned_addr) + 1024*16 + nbytes; //get alligned pointer to end of allocation block -> this will be the thread's stack pointer

    new_thread->sp = thread_stack;

    void* args_copy;
    //copy args buffer on base of initial stack
    if(nbytes > 0){
        int offset = ((nbytes + 0x0F)&~15); //get a 16 alligned address to copy args buffer to
        new_thread->sp = (char*)new_thread->sp - offset;
        memcpy(new_thread->sp, args , nbytes); //copy args buffer into top of the stack
        args_copy = new_thread->sp;
    }

    //init stack frame:

    //ret address to _thrstart
    new_thread->sp =(char*)new_thread->sp - 16; //-> set to new 16 alligned mem address	
   *(unsigned long*)new_thread->sp = (unsigned long)_thrstart;
    //write old_ebp
    new_thread->sp =(char*)new_thread->sp - 4;	
   *(unsigned long*)new_thread->sp = (unsigned long)(((char*)new_thread->sp)-3); 
    //arguement 2
    new_thread->sp =(char*)new_thread->sp - 4;	
   *(unsigned long*)new_thread->sp = (unsigned long)args;
   //arguement 1
   new_thread->sp =(char*)new_thread->sp - 4;	
   *(unsigned long*)new_thread->sp = (unsigned long)func;
   //null
   new_thread->sp =(char*)new_thread->sp - 4;	


    nthreads++;
    enqueue(&ready_queue,(void*)new_thread);
    return (int)new_thread;
}


int Thread_self(void){
    assert(current_thread!=NULL);
    return (int)current_thread;
}



void Thread_exit(int code) {
    assert(current_thread);
    cleanup_threads();
    enqueue(&free_queue,current_thread);
    nthreads--;

    switch_thread();
}


void cleanup_threads(){
    while(!isEmpty(&free_queue)){
        void* finished_thread = peek(&free_queue);
        dequeue(&free_queue);
        free(finished_thread);
    }
}