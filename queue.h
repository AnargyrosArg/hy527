#ifndef QUEUE_INCLUDED
#define QUEUE_INCLUDED

#include <stdbool.h>
#include <stdio.h>


#define MAX_SIZE 1000

typedef void* queue_item_t;

typedef struct Queue_s{
    //TODO dynamic capacity!
    queue_item_t items[MAX_SIZE];
    int front;
    int rear;
 } Queue;



void initializeQueue(Queue* q);
bool isEmpty(Queue* q);
bool isFull(Queue* q);
void enqueue(Queue* q, queue_item_t value);
void dequeue(Queue* q);
queue_item_t peek(Queue* q);
void printQueue(Queue* q);

#endif