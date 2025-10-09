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
        static void destroy(BTNode* p);
        int height(BTNode* p) const;
    protected:
        static BTNode* buildNode(int key);
    public:
        BTNode* rootNode;
        BinaryTree();
        virtual ~BinaryTree();
        void clear();
        BTNode* root() const;
        void buildTree(const int* arr, int n, int null);
        int height() const;
    };
}
#endif //BINARYTREE_H
