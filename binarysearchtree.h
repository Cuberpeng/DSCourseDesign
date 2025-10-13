//
// Created by xiang on 25-10-9.
//

#ifndef BINARYSEARCHTREE_H
#define BINARYSEARCHTREE_H
#include "binarytree.h"
namespace ds {
    class BinarySearchTree : public BinaryTree {
        static BTNode* insertNode(BTNode* root,int key);
        static BTNode* findMin(BTNode* root);
        static BTNode* findMax(BTNode* root);
        static BTNode* erase(BTNode* root, int key);
    public:
        BinarySearchTree();
        void insert(int key);
        BTNode* find(int key);
        void eraseKey(int key);
    };
}


#endif //BINARYSEARCHTREE_H
