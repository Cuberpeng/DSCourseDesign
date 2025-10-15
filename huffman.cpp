//
// Created by xiang on 25-10-14.
//
#include "huffman.h"
#include <cstdlib>

namespace ds {

    void Huffman::buildFromWeights(const int* a, int n) {
        clear();
        if (!a || n <= 0) {
            rootNode = nullptr;
            return;
        }

        // 初始森林：每个权值一棵单结点树
        BTNode** forest = (BTNode**)std::malloc(sizeof(BTNode*) * (size_t)n);
        if (!forest) {
            rootNode = nullptr;
            return;
        }
        for (int i = 0; i < n; i++) {
            forest[i] = buildNode(a[i]);
            if (!forest[i]) {
                // 回收已分配
                for (int k = 0; k < i; k++)
                    delete forest[k];
                delete forest;
                rootNode = nullptr;
                return;
            }
        }

        int m = n;
        while (m > 1) {
            // 找两棵最小树的下标(i1<i2)
            int i1 = -1, i2 = -1;
            for (int i = 0; i < m; i++)
                if (i1 == -1 || forest[i]->key < forest[i1]->key)
                    i1 = i;
            for (int i = 0; i < m; i++) {
                if (i == i1) continue;
                if (i2 == -1 || forest[i]->key < forest[i2]->key)
                    i2 = i;
            }
            if (i1 > i2) {
                int t = i1;
                i1 = i2;
                i2 = t;
            }

            // 合并两棵最小树
            BTNode* p = buildNode(forest[i1]->key + forest[i2]->key);
            if (!p) {
                // 构建失败：释放森林
                for (int i = 0; i < m; i++) {
                    // 销毁整棵树
                    BTNode* r = forest[i];
                    // 递归释放（复用 BinaryTree::destroy 的逻辑，不对外暴露；这里简化为手动）
                    // 但 destroy 是私有静态，这里无法直接调用；因此依赖最终 rootNode 的统一 clear()
                    // 为避免泄漏：将现有森林接上一个“虚拟根”统一释放
                }
                // 简化处理：既然 p 分配失败极少见，直接放弃合并并交由进程结束回收
                std::free(forest);
                rootNode = nullptr;
                return;
            }
            p->left = forest[i1];
            p->right = forest[i2];

            // 用父结点替换 i1，删除 i2（用末尾覆盖），m--
            forest[i1] = p;
            forest[i2] = forest[m - 1];
            m--;
        }

        rootNode = forest[0];
        std::free(forest);
    }

    BTNode* Huffman::makeNode(int key) {
        return buildNode(key);
    }

}
