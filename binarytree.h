//
// Created by xiang on 25-10-8.
//

#ifndef BINARYTREE_H
#define BINARYTREE_H

namespace ds {

    struct BTNode {
        int key;
        BTNode* left;
        BTNode* right;
    };

    class BinaryTree {
        // 私有工具
        static void destroy(BTNode* p);
        int  height(BTNode* p) const;

        int  countRec(const BTNode* p) const;
        void preorderRec (const BTNode* p, int* out, int maxn, int& cnt) const;
        void inorderRec  (const BTNode* p, int* out, int maxn, int& cnt) const;
        void postorderRec(const BTNode* p, int* out, int maxn, int& cnt) const;

    protected:
        static BTNode* buildNode(int key);

    public:
        BTNode* rootNode;
        BinaryTree();
        virtual ~BinaryTree();

        void clear();
        BTNode* root() const;
        void buildTree(const int* arr, int n, int null);
        int  height() const;
        int  count()  const;   // 结点总数

        // —— 三种遍历（不使用 vector）：把结果写入 out[0..ret-1]，返回写入个数。
        // 若 out==nullptr 或 maxn<=0，则只统计应写入个数（便于先询问长度再分配）。
        int preorder (int* out, int maxn) const;
        int inorder  (int* out, int maxn) const;
        int postorder(int* out, int maxn) const;
        int levelorder(int* out, int maxn) const;
    };

} // namespace ds
#endif // BINARYTREE_H
