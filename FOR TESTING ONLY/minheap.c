#include "minheap.h"
#include <math.h>
//MinHeap ADT Type Defintions

/*================= createheap ================
Allocates memory for a heap from dynamic memory 
and returns its address to the caller.
*/
MINHEAP *createheap(void) //(int maxsize)
{
    MINHEAP *heap;
    heap = (MINHEAP *)malloc(sizeof(MINHEAP));
    if (heap)
    {
        //heap->items = (MINHEAP_NODE**)malloc(sizeof(MINHEAP_NODE*));
        heap->root = NULL;
        heap->count = 0;
        //heap->maxcount = maxsize;
    }
    return heap;
}

void swap(MINHEAP_NODE *heapnode1, MINHEAP_NODE *heapnode2)
{
    MINHEAP_NODE *temp = heapnode1;
    heapnode1 = heapnode2;
    heapnode2 = temp;
}

MINHEAP_NODE *getparent(MINHEAP *heap, MINHEAP_NODE *childnode)
{
    if (heap->root == NULL || childnode == NULL)
        return NULL;
    return getparentrec(heap->root, childnode);
}

MINHEAP_NODE *getparentrec(MINHEAP_NODE *ancestornode, MINHEAP_NODE *childnode)
{
    if (ancestornode == NULL || childnode == NULL)
        return NULL;
    if (ancestornode->leftchild == childnode || ancestornode->rightchild == childnode)
        return ancestornode;
    MINHEAP_NODE *templ = getparentrec(ancestornode->leftchild, childnode);
    MINHEAP_NODE *tempr = getparentrec(ancestornode->rightchild, childnode);
    if (templ != NULL)
        return templ;
    else if (tempr != NULL)
        return tempr;
    return NULL;
}

MINHEAP_NODE *getleftchild(MINHEAP_NODE *parentnode)
{
    if (parentnode)
        return parentnode->leftchild;
}

MINHEAP_NODE *getrightchild(MINHEAP_NODE *parentnode)
{
    if (parentnode)
        return parentnode->rightchild;
}

int minheapcount(MINHEAP *heap)
{
    return heap->count;
}

// int minheapmaxcount(MINHEAP *heap)
// {
//     return heap->maxcount;
// }

void **getmin(MINHEAP *heap, void **itemPtr)
{
    if (heap->root)
        itemPtr = heap->root->dataPtr;
    else
        itemPtr = NULL;
}

bool insert(MINHEAP *heap, void **itemPtr)
{
    heap->count++;
    MINHEAP_NODE *temp = heap->root;
    int currentlevel = floor(log(heap->count) / log(2));
    int maxlevelnodes = pow(2, currentlevel);
    int maxtotalnodes = pow(2, currentlevel + 1) - 1;
    int insertposition = maxlevelnodes - (maxtotalnodes - heap->count);
    while (currentlevel > 0)
    {
        if (insertposition % 2 == 0)
        {
            temp = getrightchild(temp);
        }
        else
        {
            temp = getleftchild(temp);
        }
        int percentage = insertposition / maxlevelnodes;
        maxlevelnodes = maxlevelnodes / 2;
        insertposition = ceil(percentage * maxlevelnodes);
        currentlevel--;
    }
    temp = (MINHEAP_NODE *)malloc(sizeof(MINHEAP_NODE));
    if (temp != NULL)
    {
        temp->dataPtr = itemPtr;
        temp->leftchild = NULL;
        temp->rightchild = NULL;
        return true;
    }
    return false;
}

void reheapup(MINHEAP *heap, MINHEAP_NODE *newnode)
{
}

void reheapdown(MINHEAP *heap, MINHEAP_NODE *newnode)
{
}

bool removemin(MINHEAP *heap, void **itemPtr)
{
    return true; //for now
}

// For Integers only

// void print(MINHEAP *heap)
// {
//     if (heap->root == NULL)
//         return;
//     printrec(heap->root);
// }

// void printrec(MINHEAP_NODE *root)
// {
//     if (root == NULL)
//         return;
//     printf("%d ->");
// }

MINHEAP *destroyQueue(MINHEAP *heap)
{
    MINHEAP *deletePtr;
    if (heap)
    {
        void **itemPtr = NULL;
        while (removemin(heap, itemPtr))
        {
            ;
        }
        free(heap);
    }
    return NULL;
}
