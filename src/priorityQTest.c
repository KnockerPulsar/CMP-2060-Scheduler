#include <stdio.h>
#include "queue.c"

int compare_ints(void *a, void *b)
{
	int aPtr = *(int *)a, bPtr = *(int *)b;
	if (aPtr > bPtr)
		return 1;
	else if (aPtr < bPtr)
		return -1;
	else
		return 0;
}
void PrintQ(QUEUE *q)
{
	QUEUE_NODE *ptr = q->front;

	while (ptr)
	{
		printf("%d ", *(int *)ptr->dataPtr);
		fflush(stdout);
		ptr = ptr->next;
	}
	printf("\n");
}

int main()
{
	int num = 69;
	int numbers[] = {4, 5, 7, 11, 12, 17, 18, 19, 20, 21, 23, 24, 25, 26, 28, 30, 32, 33, 36, 37, 38, 42, 45, 48, 49, 50, 54, 56, 58, 59, 60, 61, 62, 63, 64, 65, 70, 71, 72, 73, 75, 78, 79, 81, 82, 83, 84, 87, 88, 89, 90, 91, 92, 93, 96, 97, 98, 99, 100, 102, 104, 105, 109, 110, 113, 114, 115, 118, 120};
	QUEUE *q = createQueue();
	for (size_t i = 0; i < num; i++)
	{
		enqueue_sorted(q, &numbers[i], compare_ints);
		PrintQ(q);
	}
	int * dQ;
	for (int i = 0; i < num; i++)
	{
		dequeue(q,(void*)&dQ);
		printf("Dequeued %d \n", *dQ);
	}
	printf("Finish dequeuing \n");
	
}
