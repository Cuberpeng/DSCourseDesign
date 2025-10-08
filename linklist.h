//
// Created by xiang on 25-10-8.
//

#ifndef LINKLIST_H
#define LINKLIST_H

namespace ds {
    struct LLNode {
        int value;
        LLNode *next;
    };

    class Linklist {
        LLNode *head;
        LLNode *tail;
        int length;
        LLNode* build(int v);//新建节点

        public:
        Linklist();
        ~Linklist();
        int size() const;//长度
        LLNode* gethead() const;//只读头指针

        void clear();
        void push_front(int value);
        void push_back(int value);
        bool insert(int position,int value);
        bool erase(int position);
        int get(int position) const;


    };
}

#endif //LINKLIST_H
