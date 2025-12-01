//
// Created by xiang on 25-11-6.
//

#ifndef AVL_H
#define AVL_H
#include "binarytree.h"
#include <vector>
namespace ds {
class AVL : public BinaryTree {
public:
    // 一次插入过程中，可能发生 0~多次旋转，这里记录每一次
    struct RotationRecord {
        enum Type { LL, RR, LR, RL } type;
        BTNode* z;  // 失衡结点（子树根）
        BTNode* y;  // z 的孩子（长高一侧）
        BTNode* x;  // 插入路径上的孙子（可能为 nullptr）
    };

    AVL() = default;
    // 对外插入接口：每次插入前先清空旋转记录
    void insert(int key) {
        rotationRecords_.clear();
        insert_(rootNode, key);
    }

    // 读取本次插入过程中记录下来的旋转信息（只读）
    const std::vector<RotationRecord>& rotationRecords() const { return rotationRecords_; }

private:
    std::vector<RotationRecord> rotationRecords_;

    // 计算高度：这里直接递归计算，节点数量不大，足够用
    static int height(BTNode* t) {
        if (!t) return 0;
        int hl = height(t->left);
        int hr = height(t->right);
        return (hl > hr ? hl : hr) + 1;
    }

    void recordRotation(RotationRecord::Type t, BTNode* z, BTNode* y, BTNode* x) {
        RotationRecord rec;
        rec.type = t;
        rec.z = z;
        rec.y = y;
        rec.x = x;
        rotationRecords_.push_back(rec);
    }

    // 单旋 & 双旋（标准 AVL 实现）
    static void singleRotateLeft(BTNode*& k2) {
        BTNode* k1 = k2->right;
        k2->right = k1->left;
        k1->left = k2;
        k2 = k1;
    }

    static void singleRotateRight(BTNode*& k2) {
        BTNode* k1 = k2->left;
        k2->left = k1->right;
        k1->right = k2;
        k2 = k1;
    }

    static void doubleRotateLeft(BTNode*& k3) {
        singleRotateRight(k3->right);
        singleRotateLeft(k3);
    }

    static void doubleRotateRight(BTNode*& k3) {
        singleRotateLeft(k3->left);
        singleRotateRight(k3);
    }

    // 递归插入 + 失衡检测 + 旋转记录
    void insert_(BTNode*& rt, int key) {
        if (!rt) {
            // BinaryTree 提供的 buildNode 是 protected 静态函数，子类可以直接调用
            rt = buildNode(key);
            return;
        }

        if (key < rt->key) {
            insert_(rt->left, key);

            if (height(rt->left) - height(rt->right) == 2) {
                if (key < rt->left->key) {
                    // LL：对 rt 右旋
                    BTNode* y = rt->left;
                    BTNode* x = y ? y->left : nullptr;
                    recordRotation(RotationRecord::LL, rt, y, x);
                    singleRotateRight(rt);
                } else {
                    // LR：先对左子树左旋，再对 rt 右旋
                    BTNode* y = rt->left;
                    BTNode* x = y ? y->right : nullptr;
                    recordRotation(RotationRecord::LR, rt, y, x);
                    doubleRotateRight(rt);
                }
            }
        } else if (key > rt->key) {
            insert_(rt->right, key);

            if (height(rt->right) - height(rt->left) == 2) {
                if (key > rt->right->key) {
                    // RR：对 rt 左旋
                    BTNode* y = rt->right;
                    BTNode* x = y ? y->right : nullptr;
                    recordRotation(RotationRecord::RR, rt, y, x);
                    singleRotateLeft(rt);
                } else {
                    // RL：先对右子树右旋，再对 rt 左旋
                    BTNode* y = rt->right;
                    BTNode* x = y ? y->left : nullptr;
                    recordRotation(RotationRecord::RL, rt, y, x);
                    doubleRotateLeft(rt);
                }
            }
        }
    }
};

} // namespace ds

#endif // AVL_H

