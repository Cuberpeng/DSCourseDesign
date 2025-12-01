//
// Created by xiang on 25-10-8.
//

#ifndef BINARYTREE_H
#define BINARYTREE_H

#include <cstdlib>   // malloc/free

namespace ds {

    struct BTNode {
        int key;
        BTNode* left;
        BTNode* right;
    };

    class BinaryTree {
        // 递归释放
        static void destroy(BTNode* p) {
            if (!p) return;
            destroy(p->left);
            destroy(p->right);
            std::free(p);
        }

        // 递归高度
        int height(BTNode* p) const {
            if (!p) return 0;
            int lh = height(p->left);
            int rh = height(p->right);
            return (lh > rh ? lh : rh) + 1;
        }

        // 递归计数
        int countRec(const BTNode* p) const {
            if (!p) return 0;
            return 1 + countRec(p->left) + countRec(p->right);
        }

        // 三种遍历递归
        void preorderRec(const BTNode* p, int* out, int maxn, int& cnt) const {
            if (!p) return;
            if (out && cnt < maxn) out[cnt] = p->key;
            ++cnt;
            preorderRec(p->left,  out, maxn, cnt);
            preorderRec(p->right, out, maxn, cnt);
        }

        void inorderRec(const BTNode* p, int* out, int maxn, int& cnt) const {
            if (!p) return;
            inorderRec(p->left,  out, maxn, cnt);
            if (out && cnt < maxn) out[cnt] = p->key;
            ++cnt;
            inorderRec(p->right, out, maxn, cnt);
        }

        void postorderRec(const BTNode* p, int* out, int maxn, int& cnt) const {
            if (!p) return;
            postorderRec(p->left,  out, maxn, cnt);
            postorderRec(p->right, out, maxn, cnt);
            if (out && cnt < maxn) out[cnt] = p->key;
            ++cnt;
        }

    protected:
        // 造一个新结点
        static BTNode* buildNode(int key) {
            BTNode* p = static_cast<BTNode*>(std::malloc(sizeof(BTNode)));
            if (!p) return nullptr;
            p->key  = key;
            p->left = p->right = nullptr;
            return p;
        }

    public:
        BTNode* rootNode;
        BinaryTree() : rootNode(nullptr) {}
        virtual ~BinaryTree() { clear(); }

        void clear() { destroy(rootNode); rootNode = nullptr; }
        BTNode* root() const { return rootNode; }
        // 用层序数组建树，null 表示空结点
        void buildTree(const int* arr, int n, int null) {
            clear();
            if (!arr || n <= 0) return;
            if (arr[0] == null) {
                rootNode = nullptr;
                return;
            }

            BTNode** q = static_cast<BTNode**>(
                    std::malloc(sizeof(BTNode*) * static_cast<std::size_t>(n)));
            if (!q) return;

            int head = 0, tail = 0, idx = 1;

            rootNode = buildNode(arr[0]);
            if (!rootNode) {
                std::free(q);
                return;
            }
            q[tail++] = rootNode;

            while (head < tail && idx < n) {
                BTNode* p = q[head++];

                // 左孩子
                if (idx < n && arr[idx] != null) {
                    p->left = buildNode(arr[idx]);
                    if (!p->left) {
                        std::free(q);
                        return;
                    }
                    q[tail++] = p->left;
                }
                ++idx;

                // 右孩子
                if (idx < n && arr[idx] != null) {
                    p->right = buildNode(arr[idx]);
                    if (!p->right) {
                        std::free(q);
                        return;
                    }
                    q[tail++] = p->right;
                }
                ++idx;
            }
            std::free(q);
        }

        // 高度
        int height() const { return height(rootNode); }

        // 结点总数
        int count() const { return countRec(rootNode); }

        // 先序遍历，返回写入个数；out==nullptr 或 maxn<=0 时仅返回应写入个数
        int preorder(int* out, int maxn) const {
            int cnt = 0;
            preorderRec(rootNode, out, maxn, cnt);
            return cnt;
        }

        // 中序遍历
        int inorder(int* out, int maxn) const {
            int cnt = 0;
            inorderRec(rootNode, out, maxn, cnt);
            return cnt;
        }

        // 后序遍历
        int postorder(int* out, int maxn) const {
            int cnt = 0;
            postorderRec(rootNode, out, maxn, cnt);
            return cnt;
        }

        // 层序遍历
        int levelorder(int* out, int maxn) const {
            if (!rootNode) return 0;

            int nodeCount = count();
            if (out == nullptr || maxn <= 0) {
                return nodeCount;
            }
            if (nodeCount <= 0) return 0;

            BTNode** q = static_cast<BTNode**>(
                    std::malloc(sizeof(BTNode*) * static_cast<std::size_t>(nodeCount)));
            if (!q) return 0;

            int head = 0, tail = 0;
            int cnt  = 0;

            q[tail++] = rootNode;

            while (head < tail && cnt < maxn) {
                BTNode* p = q[head++];

                out[cnt++] = p->key;

                if (p->left)  q[tail++] = p->left;
                if (p->right) q[tail++] = p->right;
            }

            std::free(q);
            return cnt;
        }
    };

} // namespace ds

#endif // BINARYTREE_H
