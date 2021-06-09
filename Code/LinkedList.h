#ifndef LINKED_LIST_H
#define LINKED_LIST_H
#include <stdlib.h>
#include <stdbool.h>

//*This Linked List is from Data Structures: A PseudocodeApproach with C,Second Edition*//
//* The refrence we used in the first year in college*//

//List ADT Type Defintions
typedef struct NODE
{
    void *dataPtr;
    struct NODE *link;
} NODE;
typedef struct
{
    int count;
    NODE *pos;
    NODE *head;
    NODE *rear;
    int (*compare)(void *argu1, void *argu2);
} LIST;

//Prototype Declarations
LIST *createList(int (*compare)(void *argu1, void *argu2));
LIST *destroyList(LIST *list);

//DONT USE addnode
int addNode(LIST *pList, void *dataInPtr);
bool removeNode(LIST *pList,
                void *keyPtr,
                void **dataOutPtr);
bool searchList(LIST *pList,
                void *pArgu,
                void **pDataOut);

bool retrieveNode(LIST *pList,
                  void *pArgu,
                  void **dataOutPtr);
bool traverse(LIST *pList,
              int fromWhere,
              void **dataOutPtr);
int listCount(LIST *pList);
bool emptyList(LIST *pList);
bool fullList(LIST *pList);

//USE _insert function
bool _insert(LIST *pList,
             NODE *pPre,
             void *dataInPtr);

// USE _delete function
void _delete(LIST *pList,
             NODE *pPre,
             NODE *pLoc,
             void **dataOutPtr);

//better search with yourself instead of this function
bool _search(LIST *pList,
             NODE **pPre,
             NODE **pLoc,
             void *pArgu);
//End of List ADT Definitions


NODE* get_before_node(LIST* theList,NODE* givenNode);

#endif