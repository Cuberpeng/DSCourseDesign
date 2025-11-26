//
// Created by xiang on 25-10-8.
//
#ifndef STACK_H
#define STACK_H

#include <cstdlib>

namespace ds {

    class Stack {
        int *a;   // 数据指针
        int top;  // 栈顶下标(元素个数)
        int cap;  // 容量

        void grow(int want) {
            int c = (cap > 0) ? cap * 2 : 8;
            if (c < want) c = want;

            int *element = static_cast<int*>(std::malloc(c * sizeof(int)));
            if (!element) return;

            for (int i = 0; i < top; ++i) {
                element[i] = a[i];
            }
            if (a) std::free(a);
            a = element;
            cap = c;
        }

    public:
        Stack() : a(nullptr), top(0), cap(0) {}

        ~Stack() {
            if (a) std::free(a);
        }

        // 入栈
        bool push(int value) {
            if (top + 1 > cap) {
                grow(top + 1);
            }
            if (top + 1 > cap) return false;
            a[top++] = value;
            return true;
        }

        // 出栈
        bool pop(int* out) {
            if (top <= 0) return false;
            --top;
            if (out) *out = a[top];
            return true;
        }

        // 读栈顶
        bool peek(int* out) const {
            if (top <= 0) return false;
            if (out) *out = a[top - 1];
            return true;
        }

        // 栈大小
        int size() const {
            return top;
        }

        // 是否为空
        bool empty() const {
            return top == 0;
        }

        // 清空栈（不释放容量）
        void clear() {
            top = 0;
        }

        // 获取第 i 个元素（从底到顶），不合法返回 0
        int get(int i) const {
            if (i < 0 || i >= top) return 0;
            return a[i];
        }

        // 获取栈顶元素，不存在返回 0
        int getPeek() const {
            return top > 0 ? a[top - 1] : 0;
        }
    };

} // namespace ds

#endif // STACK_H
