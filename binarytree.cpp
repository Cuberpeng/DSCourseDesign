//
// Created by xiang on 25-10-9.
//
#include "binarytree.h"
#include <cstdlib>   // malloc/free

namespace ds {

    BinaryTree::BinaryTree():rootNode(nullptr){}
    BinaryTree::~BinaryTree() { clear(); }

    BTNode *BinaryTree::buildNode(int key) {
        BTNode* p = (BTNode*)std::malloc(sizeof(BTNode));
        if(!p) return nullptr;
        p->key = key;
        p->left = p->right = nullptr;
        return p;
    }

    void BinaryTree::destroy(BTNode *p) {
        if (!p) return;
        destroy(p->left);
        destroy(p->right);
        std::free(p);
    }

    void BinaryTree::clear() {
        destroy(rootNode);
        rootNode = nullptr;
    }

    BTNode *BinaryTree::root() const { return rootNode; }

    // 层序数组建树（arr 为层序，null 为哨兵）
    void BinaryTree::buildTree(const int* arr, int n, int null) {
        clear();
        if (!arr || n <= 0) return;
        if (arr[0] == null) { rootNode = nullptr; return; }

        // 简易队列（指针数组），不依赖 STL 容器
        BTNode** q = (BTNode**)std::malloc(sizeof(BTNode*) * (size_t)n);
        if(!q) return;
        int head = 0, tail = 0, idx = 1;

        rootNode = buildNode(arr[0]);
        if(!rootNode){ std::free(q); return; }
        q[tail++] = rootNode;

        while(head < tail && idx < n){
            BTNode* p = q[head++];

            // 左孩子
            if (idx < n) {
                if (arr[idx] != null){
                    p->left = buildNode(arr[idx]);
                    if (p->left) q[tail++] = p->left;
                }
                ++idx;
            }
            // 右孩子
            if (idx < n) {
                if (arr[idx] != null){
                    p->right = buildNode(arr[idx]);
                    if (p->right) q[tail++] = p->right;
                }
                ++idx;
            }
        }
        std::free(q);
    }

    int BinaryTree::height(BTNode* p) const {
        if (!p) return 0;
        int lh = height(p->left);
        int rh = height(p->right);
        return (lh > rh ? lh : rh) + 1;
    }

    int BinaryTree::height() const { return height(rootNode); }

    // ===== 私有递归辅助（不使用额外 namespace）=====
    int BinaryTree::countRec(const BTNode* p) const {
        if (!p) return 0;
        return 1 + countRec(p->left) + countRec(p->right);
    }

    void BinaryTree::preorderRec(const BTNode* p, int* out, int maxn, int& cnt) const {
        if(!p) return;
        if (out && cnt < maxn) out[cnt] = p->key; ++cnt;
        preorderRec(p->left,  out, maxn, cnt);
        preorderRec(p->right, out, maxn, cnt);
    }

    void BinaryTree::inorderRec(const BTNode* p, int* out, int maxn, int& cnt) const {
        if(!p) return;
        inorderRec(p->left,  out, maxn, cnt);
        if (out && cnt < maxn) out[cnt] = p->key; ++cnt;
        inorderRec(p->right, out, maxn, cnt);
    }

    void BinaryTree::postorderRec(const BTNode* p, int* out, int maxn, int& cnt) const {
        if(!p) return;
        postorderRec(p->left,  out, maxn, cnt);
        postorderRec(p->right, out, maxn, cnt);
        if (out && cnt < maxn) out[cnt] = p->key; ++cnt;
    }

    // ===== 对外接口 =====
    int BinaryTree::count() const {
        return countRec(rootNode);
    }

    int BinaryTree::preorder(int* out, int maxn) const {
        int cnt = 0;
        preorderRec(rootNode, out, maxn, cnt);
        return cnt;
    }

    int BinaryTree::inorder(int* out, int maxn) const {
        int cnt = 0;
        inorderRec(rootNode, out, maxn, cnt);
        return cnt;
    }

    int BinaryTree::postorder(int* out, int maxn) const {
        int cnt = 0;
        postorderRec(rootNode, out, maxn, cnt);
        return cnt;
    }

} // namespace ds
