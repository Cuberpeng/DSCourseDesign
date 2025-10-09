//
// Created by xiang on 25-10-9.
//
#include "binarytree.h"
#include <cstdlib>
using namespace std;
namespace ds {
    BinaryTree::BinaryTree():rootNode(nullptr){}
    BinaryTree::~BinaryTree() {
        clear();
    }
    BTNode *BinaryTree::buildNode(int key) {
        BTNode* p=(BTNode*)malloc(sizeof(BTNode));
        if(!p)
            return nullptr;
        p->key = key;
        p->left = p->right = nullptr;
        return p;
    }
    void BinaryTree::destroy(BTNode *p) {
        if (!p)
            return;
        destroy(p->left);
        destroy(p->right);
        free(p);
    }
    void BinaryTree::clear() {
        destroy(rootNode);
        rootNode = nullptr;
    }
    BTNode *BinaryTree::root() const {
        return rootNode;
    }
    void BinaryTree::buildTree(const int* arr, int n, int null) {
        clear();
        if (!arr || n <= 0)
            return;
        if (arr[0] == null) {//空树
            rootNode = nullptr;
            return;
        }
        BTNode** q = (BTNode**)malloc(sizeof(BTNode*)*(size_t)n);
        if(!q) return;
        int head=0, tail=0, idx=1;

        rootNode = buildNode(arr[0]);
        if (!rootNode) {
            free(q);
            return;
        }
        q[tail++] = rootNode;

        while (head < tail && idx < n) {
            BTNode* p = q[head++];

            //左孩子
            if (idx < n) {
                if (arr[idx] != null){
                    p->left = buildNode(arr[idx]);
                    if (p->left)
                        q[tail++] = p->left;
                }
                idx++;
            }
            //右孩子
            if (idx < n) {
                if (arr[idx] != null){
                    p->right = buildNode(arr[idx]);
                    if (p->right)
                        q[tail++] = p->right;
                }
                idx++;
            }
        }
        free(q);
    }






}
