//
// Created by xiang on 25-10-9.
//
#ifndef BINARYSEARCHTREE_H
#define BINARYSEARCHTREE_H
#include "binarytree.h"
#include <cstdlib>
namespace ds {
    class BinarySearchTree : public BinaryTree {
        static BTNode* insertNode(BTNode* root, int key) {
            if (!root) return buildNode(key);
            if (key < root->key) root->left = insertNode(root->left, key);
            else if (key > root->key) root->right = insertNode(root->right, key);
            // 等于时不插入，保持 BST 不含重复
            return root;
        }

        static BTNode* findMin(BTNode* root) {
            if (!root) return nullptr;
            while (root->left) root = root->left;
            return root;
        }

        static BTNode* findMax(BTNode* root) {
            if (!root) return nullptr;
            while (root->right) root = root->right;
            return root;
        }

        static BTNode* erase(BTNode* root, int key) {
            if (!root) return nullptr;

            if (key < root->key) root->left = erase(root->left, key);
            else if (key > root->key) root->right = erase(root->right, key);
            else {
                // 找到要删除的结点
                if (!root->left && !root->right) {
                    std::free(root);
                    return nullptr;
                } else if (!root->left) {
                    BTNode* temp = root->right;
                    std::free(root);
                    return temp;
                } else if (!root->right) {
                    BTNode* temp = root->left;
                    std::free(root);
                    return temp;
                } else {
                    // 两个孩子：把左子树的最大结点接到右子树前面
                    BTNode* L = root->left;
                    BTNode* R = root->right;
                    BTNode* p = findMax(L);
                    p->right = R;
                    std::free(root);
                    return L;
                }
            }
            return root;
        }

    public:
        BinarySearchTree() : BinaryTree() {}

        void insert(int key) { rootNode = insertNode(rootNode, key); }
        BTNode* find(int key) {
            BTNode* p = rootNode;
            while (p) {
                if (p->key == key) return p;
                else if (key < p->key) p = p->left;
                else p = p->right;
            }
            return nullptr;
        }
        void eraseKey(int key) { rootNode = erase(rootNode, key); }
    };
} // namespace ds
#endif // BINARYSEARCHTREE_H
