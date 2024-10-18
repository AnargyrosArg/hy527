#ifndef QUEUE_INCLUDED
#define QUEUE_INCLUDED

#include <stdbool.h>
#include <stdio.h>


typedef void* queue_item_t;

typedef struct Queue_s{
    queue_item_t *items;
    int size;
    int front;
    int rear;
 } Queue;



void initializeQueue(Queue* q,int size);
bool isEmpty(Queue* q);
bool isFull(Queue* q);
void enqueue(Queue* q, queue_item_t value);
void dequeue(Queue* q);
queue_item_t peek(Queue* q);
void printQueue(Queue* q);

#endif