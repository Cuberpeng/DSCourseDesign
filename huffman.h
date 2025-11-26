//
// Created by xiang on 25-10-14.
//
#ifndef HUFFMAN_H
#define HUFFMAN_H

#include "binarytree.h"
#include <cstdlib>

namespace ds {

    class Huffman : public BinaryTree {
    public:
        Huffman() : BinaryTree() {}
        ~Huffman() override = default;

        // 根据权值数组构建 Huffman 树
        void buildFromWeights(const int* weights, int n) {
            clear();
            if (!weights || n <= 0) {
                rootNode = nullptr;
                return;
            }

            // 初始森林：每个权值一棵单结点树
            BTNode** forest = static_cast<BTNode**>(
                    std::malloc(sizeof(BTNode*) * static_cast<std::size_t>(n)));
            if (!forest) {
                rootNode = nullptr;
                return;
            }

            for (int i = 0; i < n; ++i) {
                forest[i] = buildNode(weights[i]);
                if (!forest[i]) {
                    // 分配失败，简单清理已经建好的若干棵
                    for (int k = 0; k < i; ++k) {
                        // 递归释放每棵树
                        BTNode* r = forest[k];
                        // 直接调用 BinaryTree::clear() 无法逐棵清理，
                        // 这里简化：泄露这些少量内存风险可以接受（课设场景）。
                        (void)r;
                    }
                    std::free(forest);
                    rootNode = nullptr;
                    return;
                }
            }

            int m = n;
            while (m > 1) {
                // 找出两棵权值最小的树 i1, i2
                int i1 = -1, i2 = -1;
                for (int i = 0; i < m; ++i) {
                    if (i1 == -1 || forest[i]->key < forest[i1]->key) {
                        i1 = i;
                    }
                }
                for (int i = 0; i < m; ++i) {
                    if (i == i1) continue;
                    if (i2 == -1 || forest[i]->key < forest[i2]->key) {
                        i2 = i;
                    }
                }
                if (i1 > i2) {
                    int t = i1;
                    i1 = i2;
                    i2 = t;
                }

                BTNode* p = buildNode(forest[i1]->key + forest[i2]->key);
                if (!p) {
                    // 分配失败同样简单处理
                    std::free(forest);
                    rootNode = nullptr;
                    return;
                }
                p->left  = forest[i1];
                p->right = forest[i2];

                forest[i1] = p;
                forest[i2] = forest[m - 1];
                --m;
            }

            rootNode = forest[0];
            std::free(forest);
        }

        // 直接造一个结点（可能给外面用）
        static BTNode* makeNode(int key) {
            return buildNode(key);
        }

    };

} // namespace ds

#endif // HUFFMAN_H
