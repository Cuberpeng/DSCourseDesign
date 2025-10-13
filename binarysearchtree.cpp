#include "binarysearchtree.h"

#include "binarytree.h"
#include <cstdlib>
//
// Created by xiang on 25-10-12.
//
namespace ds{
    BinarySearchTree::BinarySearchTree(){}
    BTNode* BinarySearchTree::insertNode(BTNode* root, int key) {
        if (!root) {
            return BinaryTree::buildNode(key);
        }

        if (key < root->key) {
            root->left = insertNode(root->left, key);
        }
        else if (key > root->key) {
            root->right = insertNode(root->right, key);
        }

        return root;
    }
    BTNode* BinarySearchTree::findMin(BTNode *root) {
        if (!root)
            return nullptr;
        while (root->left)
            root = root->left;
        return root;
    }
    BTNode* BinarySearchTree::findMax(BTNode *root) {
        if (!root)
            return nullptr;
        while (root->right)
            root = root->right;
        return root;
    }
    BTNode* BinarySearchTree::erase(BTNode *root, int key) {
        if (!root)
            return nullptr;
        if (key < root->key) {
            root->left = erase(root->left, key);
        }
        else if (key > root->key) {
            root->right = erase(root->right, key);
        }
        else {
            if (root->left == nullptr && root->right == nullptr) {
                delete root;
                return nullptr;
            }
            else if (root->left == nullptr) {
                BTNode* temp = root->right;
                delete root;
                return temp;
            }
            else if (root->right == nullptr) {
                BTNode* temp = root->left;
                delete root;
                return temp;
            }
            else {
                BTNode* L = root->left;
                BTNode* R = root->right;
                BTNode* p = findMax(L);
                p->right = R;
                delete root;
                return L;
            }
        }
        return root;
    }
    void BinarySearchTree::insert(int key) {
        rootNode = insertNode(rootNode, key);
    }
    BTNode* BinarySearchTree::find(int key) {
        BTNode* p = rootNode;
        while (p) {
            if (p->key == key)
                return p;
            else if (key < p->key)
                p = p->left;
            else
                p = p->right;
        }
        return nullptr;
    }
    void BinarySearchTree::eraseKey(int key) {
        rootNode = erase(rootNode, key);
    }
}