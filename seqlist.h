//
// Created by xiang on 25-9-26.
//

#ifndef SEQLIST_H
#define SEQLIST_H

#include <cstdlib>

namespace ds {

    class Seqlist {
        int *a;   // 头指针
        int n;    // 元素个数
        int cap;  // 容量

        // 扩容到至少 want 个元素
        void grow(int want) {
            int c = (cap > 0) ? cap * 2 : 8;
            if (c < want) c = want;

            int *p = static_cast<int*>(std::malloc(sizeof(int) * c));
            if (!p) return; // 申请失败，保持原状

            for (int i = 0; i < n; ++i) {
                p[i] = a[i];
            }
            if (a) std::free(a);
            a  = p;
            cap = c;
        }

    public:
        Seqlist() : a(nullptr), n(0), cap(0) {}

        ~Seqlist() {
            if (a) std::free(a);
        }

        // 元素个数
        int size() const { return n; }

        // 容量
        int capacity() const { return cap; }

        // 在 position 位置插入一个元素（0..n），成功返回 true
        bool insert(int position, int value) {
            if (position < 0 || position > n) return false;

            if (n + 1 > cap) {
                grow(n + 1);
            }
            if (n + 1 > cap) {
                // grow 失败
                return false;
            }

            // 从尾部开始右移
            for (int i = n; i > position; --i) {
                a[i] = a[i - 1];
            }
            a[position] = value;
            ++n;
            return true;
        }

        // 删除 position 位置元素（0..n-1），成功返回 true
        bool erase(int position) {
            if (position < 0 || position >= n) return false;

            for (int i = position; i < n - 1; ++i) {
                a[i] = a[i + 1];
            }
            --n;
            return true;
        }

        // 获取 position 位置元素，不合法返回 0
        int get(int position) const {
            if (position < 0 || position >= n) return 0;
            return a[position];
        }

        // 修改 position 位置元素，成功返回 true
        bool set(int position, int value) {
            if (position < 0 || position >= n) return false;
            a[position] = value;
            return true;
        }

        // 清空顺序表（不释放容量）
        void clear() {
            n = 0;
        }
    };

} // namespace ds

#endif // SEQLIST_H
