//
// Created by xiang on 25-9-26.
//
#include "seqlist.h"

namespace ds {

    Seqlist::Seqlist(): a(nullptr), n(0), cap(0) {}

    Seqlist::~Seqlist() {
        if (a) std::free(a);
    }

    int Seqlist::size() const { return n; }
    int Seqlist::capacity() const { return cap; }

    void Seqlist::grow(int want) {
        // 目标容量：至少 8，通常翻倍到 >= want
        int c = (cap > 0) ? cap * 2 : 8;
        if (c < want) c = want;

        int *p = static_cast<int*>(std::malloc(sizeof(int) * c));
        if (!p) return; // 内存申请失败则保持原状

        for (int i = 0; i < n; ++i) p[i] = a[i];
        if (a) std::free(a);
        a = p;
        cap = c;
    }

    bool Seqlist::insert(int position, int value) {
        // 合法：0..n（允许在末尾插入）
        if (position < 0 || position > n) return false;

        if (n + 1 > cap) grow(n + 1);
        if (n + 1 > cap) return false; // grow 失败

        // 从尾到 position 右移一格：写入区间 [position+1..n]
        for (int i = n; i > position; --i) {
            a[i] = a[i - 1];
        }
        a[position] = value;
        ++n;
        return true;
    }

    bool Seqlist::erase(int position) {
        // 合法：0..n-1
        if (position < 0 || position >= n) return false;

        // 从 position+1 开始左移：写入区间 [position..n-2]
        for (int i = position; i < n - 1; ++i) {
            a[i] = a[i + 1];
        }
        --n;
        return true;
    }

    int Seqlist::get(int position) const {
        // 只能访问 0..n-1（position==n 为非法）
        if (position < 0 || position >= n) return 0;
        return a[position];
    }

    bool Seqlist::set(int position, int value) {
        if (position < 0 || position >= n) return false;
        a[position] = value;
        return true;
    }

    void Seqlist::clear() { n = 0; }

} // namespace ds
