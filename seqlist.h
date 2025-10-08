//
// Created by xiang on 25-9-26.
//

#ifndef SEQLIST_H
#define SEQLIST_H
#include <cstdlib>
namespace ds {
    class Seqlist {
        int *a;//头节点
        int n;//元素个数
        int cap;//容量
        void grow(int want);

    public:
        Seqlist();
        ~Seqlist();

        int size() const;//元素个数
        int capacity() const;//容量

        bool insert(int position,int value);
        bool erase(int position);
        int get(int position) const;
        bool set(int position,int value);
        void clear();

    };
}


#endif //SEQLIST_H
