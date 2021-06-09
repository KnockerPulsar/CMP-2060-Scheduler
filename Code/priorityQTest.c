#include <stdio.h>
#include "queue.c"

int compare_ints_asc(void *a, void *b)
{
	int aPtr = *(int *)a, bPtr = *(int *)b;
	if (aPtr > bPtr)
		return -1;
	else if (aPtr < bPtr)
		return 1;
	else
		return 0;
}
int compare_ints_dsc(void *a, void *b)
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
	//int num = 7;
	int numbers[] = {182, 109, 179, 47, 92, 41, 23, 103, 65, 185, 24, 49, 8, 173, 30, 148, 166, 180, 136, 77, 198, 178, 61, 154, 100, 4, 58, 102, 45, 121, 15, 116, 36, 139, 183, 51, 147, 25, 137, 131, 66, 135, 12, 101, 122, 69, 95, 172, 14, 60, 31, 171, 169, 28, 52, 97, 124, 156, 120, 76, 160, 117, 111, 175, 13, 138, 87, 99, 194, 130, 114, 48, 192, 89, 94, 26, 10, 153, 133, 62, 157, 56, 20, 44, 68, 81, 162, 50, 184, 22, 164, 5, 78, 19, 151, 104, 144, 107, 170, 159};
	//int numbers[] = {4, 5, 2, 3, 1 , 8, 6};
	QUEUE *q = createQueue();
	printf("The input array :");
	for(int j=0; j<num; j++)
		printf(" %d",numbers[j]);
	printf("\n");
	for (size_t i = 0; i < num; i++)
	{
		enqueue_sorted(q, &numbers[i], compare_ints_dsc);
		PrintQ(q);
	}
	//int * dQ;
	//for (int i = 0; i < num; i++)
	//{
	//	dequeue(q,(void*)&dQ);
	//	printf("Dequeued %d \n", *dQ);
	//}
	//printf("Finish dequeuing \n");
	
}
