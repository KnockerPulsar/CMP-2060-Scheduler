
#include <stdio.h>
#include "minheap.h"

int main(void)
{
    printf("kak\n");
    MINHEAP *myheap;
    myheap = createheap();
    int x = 0;
    int *ptr = &x;
    insert(myheap, (void *)ptr);
    *ptr++;
    insert(myheap, (void *)ptr);
    *ptr++;
    insert(myheap, (void *)ptr);
    *ptr++;
    insert(myheap, (void *)ptr);
    *ptr++;
    insert(myheap, (void *)ptr);

    int **ptr_to_value = NULL;
    getmin(myheap, (void **)ptr_to_value);
    int temp = **ptr_to_value;
    printf("value=%d\n", temp);
    MINHEAP_NODE * myNode = getleftchild(myheap->root);

    return 0;
}