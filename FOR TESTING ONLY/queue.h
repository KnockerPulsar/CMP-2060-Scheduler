#ifndef QUEUE_H
#define QUEUE_H

#include <stdlib.h>
#include <stdbool.h>
typedef struct node
{
    void *dataPtr;
    struct node *next;
} QUEUE_NODE;
typedef struct
{
    QUEUE_NODE *front;
    QUEUE_NODE *rear;
    int count;
} QUEUE;

//Prototype Declarations
QUEUE *createQueue(void);
QUEUE *destroyQueue(QUEUE *queue);
bool dequeue(QUEUE *queue, void **itemPtr);
bool enqueue(QUEUE *queue, void *itemPtr);
bool queueFront(QUEUE *queue, void **itemPtr);
bool queueRear(QUEUE *queue, void **itemPtr);
int queueCount(QUEUE *queue);
bool emptyQueue(QUEUE *queue);
bool fullQueue(QUEUE *queue);
//End of Queue ADT Definitions

#endif