#include "BuddyBinaryTree.h"
#include "stdio.h"

BinaryTreeNode *CreateBinaryTree(void *rootData)
{
    BinaryTreeNode *root = malloc(sizeof(BinaryTreeNode));
    root->parent = NULL;
    root->dataPtr = rootData;
    return root;
}

void PreOrderTraversal(BinaryTreeNode *root, void op(BinaryTreeNode *))
{
    if (root == NULL)
        return;

    // printf("BlockSize = %d, actually allocated = %d", ((BuddySystemData*)(root->dataPtr))->blockSize, ((BuddySystemData*)(root->dataPtr))->actualAllocated);
    op(root);

    if (root->leftChild)
        PreOrderTraversal(root->leftChild, op);
    if (root->rightChild)
        PreOrderTraversal(root->rightChild, op);
}

void Split(BinaryTreeNode *nodeToSplit)
{
    BinaryTreeNode *leftChild = malloc(sizeof(BinaryTreeNode));
    BinaryTreeNode *rightChild = malloc(sizeof(BinaryTreeNode));

    BuddySystemData *leftChildData = malloc(sizeof(BuddySystemData)), *rightChildData = malloc(sizeof(BuddySystemData));

    leftChildData->blockSize = ((BuddySystemData *)(nodeToSplit->dataPtr))->blockSize / 2;
    rightChildData->blockSize = ((BuddySystemData *)(nodeToSplit->dataPtr))->blockSize / 2;

    leftChild->dataPtr = (void *)leftChildData;
    rightChild->dataPtr = (void *)rightChildData;

    leftChild->parent = nodeToSplit;
    leftChild->parent = nodeToSplit;

    nodeToSplit->leftChild = (void *)leftChild;
    nodeToSplit->rightChild = (void *)rightChild;
}

void Merge(BinaryTreeNode *leftChild, BinaryTreeNode *rightChild)
{
    free(leftChild);
    leftChild = NULL;
    free(rightChild);
    rightChild = NULL;
}

void FindSmallestFittingNode(BinaryTreeNode *root, int *size, BinaryTreeNode **returnNode)
{
    if (!root || ((BuddySystemData *)(root->dataPtr))->actualAllocated != 0)
        return;

    // If the return node hasn't been set or the current node fits and is smaller
    // Set the return node as the current node
    if (
        !(*returnNode) ||
        (((BuddySystemData *)root->dataPtr)->blockSize >= *size &&
         ((BuddySystemData *)root->dataPtr)->blockSize < ((BuddySystemData *)((*returnNode)->dataPtr))->blockSize))
        *returnNode = root;

    if (root->leftChild)
        FindSmallestFittingNode(root->leftChild, size, returnNode);
    if (root->rightChild)
        FindSmallestFittingNode(root->rightChild, size, returnNode);
}

void SplitToSmallestSize(BinaryTreeNode *root, int *reqMem, BinaryTreeNode **smallest)
{
    if (!root || ((BuddySystemData *)(root->dataPtr))->actualAllocated != 0)
        return;
    
    if(*smallest == NULL ||
        ((BuddySystemData *)(root->dataPtr))->blockSize < ((BuddySystemData *)((*smallest)->dataPtr))->blockSize
     ) *smallest = root;

    if (*reqMem <= ((BuddySystemData *)(root->dataPtr))->blockSize / 2 &&
        !root->leftChild && !root->rightChild)
    {
        Split(root);

        *smallest=root;

        SplitToSmallestSize(root->leftChild, reqMem, smallest);
        SplitToSmallestSize(root->rightChild, reqMem, smallest);
    }
}