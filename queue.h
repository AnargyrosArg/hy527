#ifndef QUEUE_INCLUDED
#define QUEUE_INCLUDED

#include <stdbool.h>
#include <stdio.h>


typedef void* queue_item_t;

typedef struct Node {
    queue_item_t* item;
    struct Node* next;
} Node;

typedef struct Queue {

    // Pointer to the front and the rear of the linked list
    Node *front, *rear;
} Queue;


void initializeQueue(Queue* q);
bool isEmpty(Queue* q);
bool isFull(Queue* q);
void enqueue(Queue* q, queue_item_t value);
void dequeue(Queue* q);
queue_item_t peek(Queue* q);
void printQueue(Queue* q);

#endif