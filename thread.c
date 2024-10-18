#include <thread.h>
#include <assert.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <signal.h>
#include <sys/time.h>
#include </usr/include/signal.h>

// 1 million max active threads -> size of free and ready queue
#define MAX_ACTIVE_THREADS 1000000

// limit size of join queue to save memory
#define MAX_JOIN_THREADS 1000

#define CRITICAL_SECTION(code)   \
    do                           \
    {                            \
        ++CRITICAL_SECTION_FLAG; \
        code;                    \
        --CRITICAL_SECTION_FLAG; \
    } while (0)

void _STARTMONITOR(void) {};


void check_duplicates(void **array,int count);

static int CRITICAL_SECTION_FLAG = 0;

/* Queue implementation inside thread.c in order to guard it inside _STARTMONITOR/_ENDMONITOR */

// Function to initialize the queue
void initializeQueue(Queue *q, int size)
{
    q->front = 0;
    q->rear = 0;
    q->size = size + 1;
    CRITICAL_SECTION(q->items = malloc((size + 1) * sizeof(queue_item_t)));
    CRITICAL_SECTION(memset(q->items,0,(size + 1) * sizeof(queue_item_t)));
}

void destroyQueue(Queue *q)
{
    CRITICAL_SECTION(free(q->items));
}

// Function to check if the queue is empty
bool isEmpty(Queue *q) { return (q->front == q->rear); }

// Function to check if the queue is full
bool isFull(Queue *q) { return (((q->rear + 1) % q->size) == q->front); }

// Function to add an element to the queue (Enqueue
// operation)
void enqueue(Queue *q, queue_item_t value)
{
    if (isFull(q))
    {
        CRITICAL_SECTION(printf("full!\n"));
        exit(-1);
    }
    q->items[q->rear] = value;
    q->rear = (q->rear + 1) % q->size;
}

// Function to remove an element from the queue (Dequeue
// operation)
void dequeue(Queue *q)
{
    if (isEmpty(q))
    {
        CRITICAL_SECTION(printf("empty!\n"));
        exit(-1);
    }
    q->front = (q->front + 1) % q->size;
}

// Function to get the element at the front of the queue
// (Peek operation)
queue_item_t peek(Queue *q)
{
    if (isEmpty(q))
    {
        CRITICAL_SECTION(printf("Queue is empty\n"));
        return (void *)-1;
    }
    return q->items[q->front];
}


/* Thread implementation */

extern void _swtch(void *from, void *to);
extern void _thrstart(void);
void cleanup_threads();
extern void _ENDMONITOR(void);

static void *current_thread = NULL; // keeps current running thread
static int nthreads = -1;           // number of active threads
static void *join_all = NULL;
Queue ready_queue;
Queue free_queue;
thread_t main_thread;
void switch_thread(void);

void preemptive_context_switch(int sig, struct sigcontext context)
{
    if (context.eip >= (unsigned long)_STARTMONITOR && context.eip <= (unsigned long)_ENDMONITOR)
    {
        return;
    }
    else if (CRITICAL_SECTION_FLAG > 0)
    {
        return;
    }

    enqueue(&ready_queue, current_thread);
    CRITICAL_SECTION(sigsetmask(context.oldmask));
    switch_thread();
    return;
}

void switch_thread(void)
{
    assert(current_thread != NULL);
    thread_t *next_thread = peek(&ready_queue);
    dequeue(&ready_queue);
    void *previous_thread = current_thread;
    current_thread = next_thread;
    _swtch(previous_thread, next_thread);
}

void Thread_init(void)
{
    if (nthreads != -1 || current_thread != NULL)
    {
        fprintf(stderr, "Thread_init on (probably) already initialized state!\n");
    }

    // init ready queue
    initializeQueue(&ready_queue, MAX_ACTIVE_THREADS);
    initializeQueue(&free_queue, MAX_ACTIVE_THREADS);
    initializeQueue(&(main_thread.join_queue), MAX_JOIN_THREADS);
    main_thread.id = (int)&main_thread;
    current_thread = &main_thread;
    nthreads = 1;

    // Pre emption
    struct sigaction sa;
    struct itimerval timer;

    // setup signal handler , no need to guard -> no threads running yet
    memset(&sa, 0, sizeof(sa));
    sa.sa_handler = (void (*)(int)) & preemptive_context_switch;
    sigaction(SIGVTALRM, &sa, NULL);

    // set timer interval
    timer.it_value.tv_sec = 0;
    timer.it_value.tv_usec = 1; // highest possible frequency to test for concurrency errors
    timer.it_interval.tv_sec = 0;
    timer.it_interval.tv_usec = 1;

    setitimer(ITIMER_VIRTUAL, &timer, NULL);

    return;
}

void Thread_pause(void)
{
    assert(current_thread != NULL);
    enqueue(&ready_queue, current_thread);
    switch_thread();
}

int Thread_new(int func(void *), void *args, size_t nbytes, ...)
{
    assert(func != NULL);
    assert(nbytes >= 0);
    if (args == NULL)
    {
        assert(nbytes == 0);
    }
    else
    {
        assert(nbytes > 0);
    }
    thread_t *new_thread;
    cleanup_threads();

    CRITICAL_SECTION(new_thread = malloc(1024 * 16 + nbytes + sizeof(thread_t) + 15)); // allocate bytes for stack + thread_t type -> +15 to be able to round up to already allocated memory
    assert(new_thread != NULL);

    // initialize join queue for thread
    initializeQueue(&(new_thread->join_queue), MAX_JOIN_THREADS);

    void *alligned_addr = (void *)(((uintptr_t)new_thread + 15) & ~(uintptr_t)0x0F); // fixes ptr to be 16 alligned
    void *thread_stack = ((char *)alligned_addr) + 1024 * 16 + nbytes;               // get alligned pointer to end of allocation block -> this will be the thread's stack pointer
    new_thread->sp = thread_stack;

    void *args_copy;
    // copy args buffer on base of initial stack
    if (nbytes > 0)
    {
        int offset = ((nbytes + 0x0F) & ~15); // get a 16 alligned address to copy args buffer to
        new_thread->sp = (char *)new_thread->sp - offset;
        CRITICAL_SECTION(memcpy(new_thread->sp, args, nbytes)); // copy args buffer into top of the stack
        args_copy = new_thread->sp;
    }

    // init stack frame:

    // ret address to _thrstart
    new_thread->sp = (char *)new_thread->sp - 16; //-> set to new 16 alligned mem address
    *(unsigned long *)new_thread->sp = (unsigned long)_thrstart;
    // write old_ebp
    new_thread->sp = (char *)new_thread->sp - 4;
    *(unsigned long *)new_thread->sp = (unsigned long)(((char *)new_thread->sp) - 3);
    // arguement 2
    new_thread->sp = (char *)new_thread->sp - 4;
    *(unsigned long *)new_thread->sp = (unsigned long)args;
    // arguement 1
    new_thread->sp = (char *)new_thread->sp - 4;
    *(unsigned long *)new_thread->sp = (unsigned long)func;
    // null
    new_thread->sp = (char *)new_thread->sp - 4;

    nthreads++;
    enqueue(&ready_queue, (void *)new_thread);
    return (int)new_thread;
}

int Thread_self(void)
{
    assert(current_thread != NULL);
    return (int)current_thread;
}

void Thread_exit(int code)
{
    assert(current_thread != NULL);
    thread_t *current = current_thread;
    cleanup_threads();
    while (!isEmpty(&(current->join_queue)))
    {
        void *continue_thread = peek(&(current->join_queue));
        dequeue(&(current->join_queue));
        enqueue(&ready_queue, continue_thread);
        ((thread_t *)continue_thread)->joined_code = code;
    }
    enqueue(&free_queue, current_thread);

    if (join_all != NULL && nthreads == 2)
    {
        enqueue(&ready_queue, join_all);
    }
    if (nthreads == 1)
    {
        nthreads--;
        cleanup_threads();
        destroyQueue(&(main_thread.join_queue));
        destroyQueue(&free_queue);
        destroyQueue(&ready_queue);
        exit(code);
    }
    nthreads--;
    switch_thread();
}

void cleanup_threads()
{
    CRITICAL_SECTION(
        while (!isEmpty(&free_queue)) {
            void *finished_thread = peek(&free_queue);
            dequeue(&free_queue);
            if (finished_thread != &main_thread)
            {
                destroyQueue(&(((thread_t *)finished_thread)->join_queue));
                CRITICAL_SECTION(free(finished_thread));
            }
        });
}

int Thread_join(int tid)
{
    assert(current_thread != (void *)tid);
    assert(current_thread != 0);
    if (tid != 0)
    {
        thread_t *awaited_thread = (void *)tid;
        enqueue(&(awaited_thread->join_queue), current_thread);
        switch_thread();
        return ((thread_t *)current_thread)->joined_code;
    }
    else if (tid == 0)
    {
        assert(join_all == NULL); // make sure no other thread is waiting all
        if (nthreads > 1)
        {
            join_all = current_thread;
            switch_thread();
        }
        return 0;
    }
}
