#include <thread.h>
#include <assert.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <signal.h>
#include <sys/time.h>
#include </usr/include/signal.h>
#include <sem.h>

#define CRITICAL_SECTION(code)   \
    do                           \
    {                            \
        ++CRITICAL_SECTION_FLAG; \
        code;                    \
        --CRITICAL_SECTION_FLAG; \
    } while (0)

void _STARTMONITOR(void) {};

void check_duplicates(void **array, int count);

static int CRITICAL_SECTION_FLAG = 0;

/* Queue implementation inside thread.c in order to guard it inside _STARTMONITOR/_ENDMONITOR */

Node* createNode(queue_item_t item)
{
    Node* new_node;
    CRITICAL_SECTION(    new_node = (Node*)malloc(sizeof(Node)));
    new_node->item = item;
    new_node->next = NULL;
    return new_node;
}

// Function to initialize the queue
void initializeQueue(Queue *q)
{
    q->front = NULL;
    q->rear = NULL;
}
// Function to check if the queue is empty
bool isEmpty(Queue* q)
{
    return q->front == NULL && q->rear == NULL;
}

// Function to add an element to the queue
void enqueue(Queue* q, queue_item_t new_data)
{
    // Create a new linked list node
    Node* new_node = createNode(new_data);

    // If queue is empty, the new node is both the front
    // and rear
    if (q->rear == NULL) {
        q->front = q->rear = new_node;
        return;
    }

    // Add the new node at the end of the queue and
    // change rear
    q->rear->next = new_node;
    q->rear = new_node;
}

// Function to remove an element from the queue
void dequeue(Queue* q)
{
    // If queue is empty, return
    if (isEmpty(q)) {
        printf("Queue Underflow\n");
        return;
    }

    // Store previous front and move front one node
    // ahead
    Node* temp = q->front;
    q->front = q->front->next;

    // If front becomes null, then change rear also
    // to null
    if (q->front == NULL)
        q->rear = NULL;

    // Deallocate memory of the old front node
    CRITICAL_SECTION(free(temp));
}


queue_item_t peek(Queue* q)
{

    // Checking if the queue is empty
    if (isEmpty(q)) {
        printf("Queue is empty\n");
        return NULL;
    }
    return q->front->item;
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

    CRITICAL_SECTION(sigsetmask(context.oldmask));
    Thread_pause();
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
    initializeQueue(&ready_queue);
    initializeQueue(&free_queue);
    initializeQueue(&(main_thread.join_queue));
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
    timer.it_value.tv_usec = 30;
    timer.it_interval.tv_sec = 0;
    timer.it_interval.tv_usec = 30;

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
    initializeQueue(&(new_thread->join_queue));

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
        join_all = NULL;
    }
    if (nthreads == 1)
    {
        nthreads--;
        cleanup_threads();
        // destroyQueue(&(main_thread.join_queue));
        // destroyQueue(&free_queue);
        // destroyQueue(&ready_queue);
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
                // destroyQueue(&(((thread_t *)finished_thread)->join_queue));
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

void Sem_init(Sem_T *sem, int count)
{
    assert(sem != NULL);

    CRITICAL_SECTION(Queue *q = malloc(sizeof(Queue));
                     initializeQueue(q);
                     sem->count = count;
                     sem->queue = q);
}

void Sem_wait(Sem_T *sem)
{
    assert(sem);
    assert(current_thread);
    if (sem->count <= 0)
    {
        enqueue(sem->queue, current_thread);
        switch_thread();
    }
    else
    {
        sem->count--;
    }
}

void Sem_signal(Sem_T *sem)
{
    assert(sem);
    assert(current_thread);

    if (sem->count == 0 && !isEmpty(sem->queue))
    {
        void *thread = peek(sem->queue);
        dequeue(sem->queue);
        enqueue(&ready_queue, thread);
    }
    else
    {
        sem->count++;
    }
}
