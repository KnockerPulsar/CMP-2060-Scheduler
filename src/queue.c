#include "queue.h"

//*This queue is from Data Structures: A PseudocodeApproach with C,Second Edition*//
//* The refrence we used in the first year in college*//

//Queue ADT Type Defintions

/*================= createQueue ================
Allocates memory for a queue head node from dynamic
memory and returns its address to the caller.
Pre nothing
Post head has been allocated and initialized
Return head if successful; null if overflow
*/
QUEUE *createQueue(void)
{
    //Local Definitions
    QUEUE *queue;
    //Statements
    queue = (QUEUE *)malloc(sizeof(QUEUE));
    if (queue)
    {
        queue->front = NULL;
        queue->rear = NULL;
        queue->count = 0;
    } // if
    return queue;
} // createQueue

/*================= enqueue ================
This algorithm inserts data into a queue.
Pre queue has been created
Post data have been inserted
Return true if successful, false if overflow
*/

bool enqueue(QUEUE *queue, void *itemPtr)
{
    //Local Definitions
    QUEUE_NODE *newPtr;
    //Statements
    if (!(newPtr = (QUEUE_NODE *)malloc(sizeof(QUEUE_NODE))))
        return false;
    newPtr->dataPtr = itemPtr;
    newPtr->next = NULL;
    if (queue->count == 0)
        // Inserting into null queue
        queue->front = newPtr;
    else
        queue->rear->next = newPtr;
    (queue->count)++;
    queue->rear = newPtr;
    return true;
} // enqueue

/*================= enqueue sorted ================
This algorithm inserts data into a queue given a comparison function to preserve sorting.

If we want the queue to be a max priority queue:
The comparison function returns 1 if left > right, 0 if left == right, -1 if left < right

Pre queue has been created
Post data have been inserted with sorting
Return true if successful, false if overflow or queue is null
*/

bool enqueue_sorted(QUEUE *queue, void *itemPtr, int comp_func(void *, void *))
{
    //Local Definitions
    QUEUE_NODE *newPtr;

    // Try to allocate memory, return false if no memory is available
    if (!(newPtr = (QUEUE_NODE *)malloc(sizeof(QUEUE_NODE))) || !queue)
        return false;

    newPtr->dataPtr = itemPtr;
    // newPtr->next = NULL;

    if (queue->count == 0)
        // Inserting into null queue
        queue->front = newPtr;
    // Just one element
    else if (queue->count == 1)
    {
        if (comp_func(queue->front->dataPtr, newPtr->dataPtr) == -1)
        {
            newPtr->next = queue->front;
            queue->front = newPtr;
        }
        else
        {
            queue->front->next = newPtr;
        }
        queue->count++;
        return true;
    }
    else
    {
        QUEUE_NODE *prevCheckElement = NULL;
        QUEUE_NODE *currCheckElement = queue->front;
        // Loop until you find the first smallest element
        for (int i = 0; i < queue->count; i++)
        {

            if (comp_func(newPtr->dataPtr, currCheckElement->dataPtr) == 1)
            {
                // For a max priority queue

                // If we have a higher value than the front, take its place
                if (prevCheckElement == NULL)
                {
                    newPtr->next = queue->front;
                    queue->front = newPtr;
                }
                // If no value in the queue is lesser, that means that we're the min
                // Insert at the end
                else if (currCheckElement == NULL)
                {
                    queue->rear->next = newPtr;
                    queue->rear = newPtr;
                }
                // If the current element is less than the element we want to insert
                // Then we should insert the new element between the previous element and the current element
                // So, prev->next = newItem, newItem->next = current
                else
                {
                    prevCheckElement->next = newPtr;
                    newPtr->next = currCheckElement;
                }
                break;
            }
            prevCheckElement = currCheckElement;
            currCheckElement = currCheckElement->next;
        }
    }

    (queue->count)++;
    return true;
} // enqueue

/*================= dequeue ================
This algorithm deletes a node from the queue.
Pre queue has been created
Post Data pointer to queue front returned and
front element deleted and recycled.
Return true if successful; false if underflow
*/
bool dequeue(QUEUE *queue, void **itemPtr)
{
    //Local Definitions
    QUEUE_NODE *deleteLoc;
    //Statements
    if (!queue->count)
        return false;
    *itemPtr = queue->front->dataPtr;
    deleteLoc = queue->front;
    if (queue->count == 1)
        // Deleting only item in queue
        queue->rear = queue->front = NULL;
    else
        queue->front = queue->front->next;
    (queue->count)--;
    free(deleteLoc);
    return true;
} // dequeue

/*================== queueFront =================
This algorithm retrieves data at front of the
queue without changing the queue contents.
Pre queue is pointer to an initialized queue
Post itemPtr passed back to caller
Return true if successful; false if underflow
*/
// mohamed khaled: itemPtr points to the data saved in the queue
//                     so if we dequeue the pointer will point to grabage(newly freed space)

bool queueFront(QUEUE *queue, void **itemPtr)
{
    //Statements
    if (!queue->count)
        return false;
    else
    {
        *itemPtr = queue->front->dataPtr;
        return true;
    } // else
} // queueFront

/*================== queueRear =================
Retrieves data at the rear of the queue
without changing the queue contents.
Pre queue is pointer to initialized queue
Post Data passed back to caller
Return true if successful; false if underflow
*/
bool queueRear(QUEUE *queue, void **itemPtr)
{
    //Statements
    if (!queue->count)
        return true;

    else
    {
        *itemPtr = queue->rear->dataPtr;
        return false;
    } // else
} // queueRear

/*================== emptyQueue =================
This algorithm checks to see if queue is empty.
Pre queue is a pointer to a queue head node
Return true if empty; false if queue has data
*/
bool emptyQueue(QUEUE *queue)
{
    //Statements
    return (queue->count == 0);
} // emptyQueue

/*================== fullQueue =================
This algorithm checks to see if queue is full. It
is full if memory cannot be allocated for next node.
Pre queue is a pointer to a queue head node
Return true if full; false if room for a node
*/
bool fullQueue(QUEUE *queue)
{

    //Local Definitions
    QUEUE_NODE *temp;
    //Statements
    temp = (QUEUE_NODE *)malloc(sizeof(*(queue->rear)));
    if (temp)
    {
        free(temp);
        return true;
    } // if
    // Heap full
    return false;
} // fullQueue

/*================== queueCount =================
Returns the number of elements in the queue.
Pre queue is pointer to the queue head node
Return queue count
*/
int queueCount(QUEUE *queue)
{
    //Statements
    return queue->count;
} // queueCount

/*================== destroyQueue =================
Deletes all data from a queue and recycles its
memory, then deletes & recycles queue head pointer.
Pre Queue is a valid queue
Post All data have been deleted and recycled
Return null pointer
*/
QUEUE *destroyQueue(QUEUE *queue)
{
    //Local Definitions
    QUEUE_NODE *deletePtr;
    //Statements
    if (queue)
    {
        while (queue->front != NULL)
        {
            free(queue->front->dataPtr);
            deletePtr = queue->front;
            queue->front = queue->front->next;
            free(deletePtr);
        } // while
        free(queue);
    } // if
    return NULL;
} // destroyQueue
