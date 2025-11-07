//
// Created by xiang on 25-11-6.
//

#ifndef AVL_H
#define AVL_H
#include "binarytree.h"

namespace ds {

    class AVL : public BinaryTree {
        static int  h(BTNode* p){
            if(!p) return 0;
            int hl = h(p->left), hr = h(p->right);
            return (hl>hr?hl:hr) + 1;
        }
        static int  bf(BTNode* p){
            if (!p) return 0;
            return h(p->left) - h(p->right);
        }
        static BTNode* rotateRight(BTNode* y){
            if (!y || !y->left) return y;
            BTNode* x = y->left;
            BTNode* T2 = x->right;
            x->right = y;
            y->left = T2;
            return x;
        }
        static BTNode* rotateLeft(BTNode* x){
            if (!x || !x->right) return x;
            BTNode* y = x->right;
            BTNode* T2 = y->left;
            y->left = x;
            x->right = T2;
            return y;
        }
        static BTNode* insert_(BTNode* rt, int key){
            if(!rt) return BinaryTree::buildNode(key);
            if(key < rt->key)
                rt->left = insert_(rt->left, key);
            else if(key > rt->key)
                rt->right = insert_(rt->right, key);
            else
                return rt; // 忽略重复

            int balance = bf(rt);

            // Left Left Case
            if (balance > 1 && key < rt->left->key)
                return rotateRight(rt);

            // Right Right Case
            if (balance < -1 && key > rt->right->key)
                return rotateLeft(rt);

            // Left Right Case
            if (balance > 1 && key > rt->left->key) {
                rt->left = rotateLeft(rt->left);
                return rotateRight(rt);
            }

            // Right Left Case
            if (balance < -1 && key < rt->right->key) {
                rt->right = rotateRight(rt->right);
                return rotateLeft(rt);
            }

            return rt;
        }

    public:
        AVL(): BinaryTree() {}
        void insert(int key){ rootNode = insert_(rootNode, key); }
    };

} // namespace ds
#endif //AVL_H
