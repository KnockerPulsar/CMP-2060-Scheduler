#ifndef MIN_HEAP
#define MIN_HEAP

#include <stdlib.h>
#include <stdbool.h>
typedef struct node
{
    void *dataPtr;
    struct node *leftchild;
    struct node *rightchild;
} MINHEAP_NODE;
typedef struct
{
    //MINHEAP_NODE **items;
    MINHEAP_NODE *root;
    int count;
    //int maxcount;
} MINHEAP;

//Prototype Declarations
MINHEAP *createheap(void);//(int maxsize);
MINHEAP *destroyQueue(MINHEAP *heap);
void swap(MINHEAP_NODE *heapnode1, MINHEAP_NODE *heapnode2);
MINHEAP_NODE * getparent(MINHEAP *heap, MINHEAP_NODE * childnode);
MINHEAP_NODE * getparentrec(MINHEAP_NODE * ancestornode, MINHEAP_NODE * childnode);
MINHEAP_NODE * getleftchild(MINHEAP_NODE * parentnode);
MINHEAP_NODE * getrightchild(MINHEAP_NODE * parentnode);
int minheapcount(MINHEAP *heap);
//int minheapmaxcount(MINHEAP *heap);
void ** getmin(MINHEAP *heap, void **itemPtr);
bool insert(MINHEAP *heap, void **itemPtr);
void reheapup(MINHEAP *heap,MINHEAP_NODE * newnode);
void reheapdown(MINHEAP *heap, MINHEAP_NODE * newnode);
bool removemin(MINHEAP *heap, void **itemPtr);
// void print(MINHEAP *heap);
// void printrec(MINHEAP_NODE * root);
//End of MINHEAP ADT Definitions

#endif