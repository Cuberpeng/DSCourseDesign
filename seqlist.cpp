//
// Created by xiang on 25-9-26.
//

#include "seqlist.h"
using namespace std;
namespace ds {
    Seqlist::Seqlist():a(nullptr),n(0),cap(0){}
    Seqlist::~Seqlist() {
        if(a!=nullptr)
            free(a);
    }

    int Seqlist::size() const {
        return n;
    }
    int Seqlist::capacity() const {
        return cap;
    }
    void Seqlist::grow(int want) {//顺序表建立
        int c = 8;
        if (cap != 0)
            c = cap * 2;
        if (c < want)
            c = want;

        int *element = (int *)malloc(c * sizeof(int));
        if (!element)
            return;

        for (int i = 0; i < n; i++) {
            element[i] = a[i];
        }
        if (a) free(a);
        a = element;
        cap = c;
    }
    bool Seqlist::insert(int position,int value) {//顺序表插入
        if (position < 0 || position > n)
            return false;
        if (n+1 > cap)
            grow(n+1);
        //if (!a && cap == 0) return false;
        for (int i = n; i > position; i--) {
            a[i] = a[i-1];
        }
        a[position] = value;
        n++;
        return true;
    }
    bool Seqlist::erase(int position) {//顺序表清除
        if (position < 0 || position > n)
            return false;
        for (int i = position; i < n; i++) {
            a[i] = a[i+1];
        }
        n--;
        return true;
    }
    int Seqlist::get(int position) const {
        if (position < 0 || position > n)
            return 0;
        else {
            return a[position];
        }
    }
    bool Seqlist::set(int position, int value) {
        if (position < 0 || position > n)
            return false;
        else {
            a[position] = value;
            return true;
        }
    }
    void Seqlist::clear() {
        n = 0;
    }

}