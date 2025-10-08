//
// Created by xiang on 25-10-8.
//

#ifndef STACK_H
#define STACK_H

namespace ds {
    class Stack {
        int *a;//数据指针
        int top;//大小
        int cap;//容量
        void grow(int want);
        public:
        Stack();
        ~Stack();
        bool push(int value);
        bool pop(int* out);
        bool peek(int* out) const;//栈顶
        int size() const;
        bool empty() const;
        void clear();
        int get(int i) const;
        int getPeek() const;//栈顶值
    };
}

#endif //STACK_H
