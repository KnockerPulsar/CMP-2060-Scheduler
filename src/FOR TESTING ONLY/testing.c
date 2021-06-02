
#include<stdio.h>
#include"queue.h"


int main(void)
{
    printf("kak\n");
    QUEUE *queue_of_integers;
    queue_of_integers=createQueue();
    int x=0;
    int *ptr=&x;
    enqueue(queue_of_integers,(void*)ptr);
    *ptr++;
    enqueue(queue_of_integers,(void*)ptr);
    *ptr++;
    enqueue(queue_of_integers,(void*)ptr);
    *ptr++;
    enqueue(queue_of_integers,(void*)ptr);
    *ptr++;

    while (!emptyQueue(queue_of_integers))
    {
        int ** ptr_to_value;
        queueFront(queue_of_integers,(void**)ptr_to_value);
        int temp= **ptr_to_value;
        dequeue(queue_of_integers,(void**)ptr_to_value);
        printf("value=%d\n",temp);
    }
    
    return 0;
}