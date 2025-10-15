//
// Created by xiang on 25-10-14.
//

#ifndef HUFFMAN_H
#define HUFFMAN_H
#include "binarytree.h"
namespace ds {
    class Huffman : public BinaryTree {
    public:
        Huffman() : BinaryTree() {}
        ~Huffman() override = default;
        void buildFromWeights(const int* weights, int n);
        static BTNode* makeNode(int key);
    };
}
#endif //HUFFMAN_H
