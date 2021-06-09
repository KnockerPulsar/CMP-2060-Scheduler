#include "BuddyBinaryTree.h"
#include "stdio.h"

void PrintBTNode(BinaryTreeNode *root)
{
    printf("BlockSize = %d, actually allocated = %d\n", ((BuddySystemData *)(root->dataPtr))->blockSize, ((BuddySystemData *)(root->dataPtr))->actualAllocated);
}

int main(int argc, char const *argv[])
    {
    BuddySystemData rootData = {1024, 0};
    BinaryTreeNode *rootNode = CreateBinaryTree((void *)&rootData);
    Split(rootNode);
    Split(rootNode->leftChild);
    Split(rootNode->rightChild);
    Split(rootNode->leftChild->leftChild);
    Split(rootNode->leftChild->rightChild);
    Split(rootNode->rightChild->leftChild);
    Split(rootNode->rightChild->rightChild);



    PreOrderTraversal(rootNode, PrintBTNode);

    return 0;
}
