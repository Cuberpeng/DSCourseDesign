//
// Created by xiang on 25-10-8.
//
#include "stack.h"
#include <stdlib.h>
namespace ds {
    Stack::Stack():a(nullptr),top(0),cap(0){}
    Stack::~Stack() {
        if (a)
            std::free(a);
    }
    void Stack::grow(int want) {
        int c = 8;
        if (cap != 0)
            c = cap * 2;
        if (c < want)
            c = want;

        int *element = (int *)malloc(c * sizeof(int));
        if (!element)
            return;

        for (int i = 0; i < top; i++) {
            element[i] = a[i];
        }
        if (a) free(a);
        a = element;
        cap = c;
    }
    bool Stack::push(int v) {
        if(top + 1 > cap)
            grow(top + 1);
        a[top++] = v;
        return true;
    }
    bool Stack::pop(int* out) {
        if(top <= 0)
            return false;
        top--;
        if(out)
            *out = a[top];
        return true;
    }
    bool Stack::peek(int* out) const {
        if(top <= 0)
            return false;
        if(out)
            *out = a[top - 1];
        return true;
    }
    int Stack::size() const {
        return top;
    }
    bool Stack::empty() const {
        return top == 0;
    }
    void Stack::clear() {
        top = 0;
    }
    int Stack::get(int i) const {
        if(i < 0 || i >= top)
            return 0;
        return a[i];
    }
    int Stack::getPeek() const {
        return top > 0 ? a[top - 1]:0;
    }

}
