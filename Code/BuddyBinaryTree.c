#include "BuddyBinaryTree.h"
#include "stdio.h"

BinaryTreeNode *CreateBinaryTree(void *rootData)
{
    BinaryTreeNode *root = malloc(sizeof(BinaryTreeNode));
    root->dataPtr = rootData;
    return root;
}

void PreOrderTraversal(BinaryTreeNode *root, void op(BinaryTreeNode*))
{
    if(root == NULL) return;

    // printf("BlockSize = %d, actually allocated = %d", ((BuddySystemData*)(root->dataPtr))->blockSize, ((BuddySystemData*)(root->dataPtr))->actualAllocated);
    op(root);

    if (root->leftChild)
        PreOrderTraversal(root->leftChild,op);
    if (root->rightChild)
        PreOrderTraversal(root->rightChild,op);
}

void Split(BinaryTreeNode *nodeToSplit)
{
    BinaryTreeNode *leftChild = malloc(sizeof(BinaryTreeNode));
    BinaryTreeNode *rightChild = malloc(sizeof(BinaryTreeNode));

    BuddySystemData *leftChildData = malloc(sizeof(BuddySystemData)), *rightChildData = malloc(sizeof(BuddySystemData));

    leftChildData->blockSize = ((BuddySystemData *)(nodeToSplit->dataPtr))->blockSize / 2;
    rightChildData->blockSize = ((BuddySystemData *)(nodeToSplit->dataPtr))->blockSize / 2;

    leftChild->dataPtr = (void*)leftChildData;
    rightChild->dataPtr = (void*)rightChildData;

    nodeToSplit->leftChild = (void*)leftChild;
    nodeToSplit->rightChild = (void*)rightChild;
}

void Merge(BinaryTreeNode *leftChild, BinaryTreeNode *rightChild)
{
    free(leftChild);
    leftChild = NULL;
    free(rightChild);
    rightChild = NULL;
}
