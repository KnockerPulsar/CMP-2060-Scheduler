#include "stdlib.h"

typedef struct BuddySystemData
{
    int blockSize, actualAllocated;
} BuddySystemData;

typedef struct BinaryTreeNode
{
    void *dataPtr;
    struct BinaryTreeNode *parent, *leftChild, *rightChild;
} BinaryTreeNode;

// Basic binary tree functions
BinaryTreeNode *CreateBinaryTree(void *rootData);
void PreOrderTraversal(BinaryTreeNode *root, void op(BinaryTreeNode*));

// Buddy system tree functions
void Split(BinaryTreeNode *nodeToSplit);
void Merge(BinaryTreeNode *leftChild, BinaryTreeNode *rightChild);
void FindSmallestFittingNode(BinaryTreeNode *root, int *size, BinaryTreeNode **returnNode);
void SplitToSmallestSize(BinaryTreeNode *root, int *reqMem, BinaryTreeNode **smallest);

