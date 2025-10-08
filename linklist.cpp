//
// Created by xiang on 25-10-8.
//
#include "linklist.h"
#include <stdlib.h>
namespace ds {
    Linklist::Linklist():head(nullptr),tail(nullptr),length(0){}
    Linklist::~Linklist() {
        clear();
    }

    LLNode *Linklist::build(int v) {
        LLNode* p=(LLNode*)std::malloc(sizeof(LLNode));
        if(!p) return nullptr;
        p->value=v;
        p->next=nullptr;
        return p;
    }

    int Linklist::size() const {
        return length;
    }
    LLNode *Linklist::gethead() const {
        return head;
    }
    void Linklist::clear() {
        LLNode* p = head;
        while(p) {
            LLNode* q = p->next;
            std::free(p);
            p=q;
        }
        head = tail = nullptr;
        length = 0;
    }
    void Linklist::push_front(int value) {//头插法
        LLNode* p = build(value);
        if(!p) return;
        p->next = head;
        head = p;
        if(!tail) tail = p;
        length++;
    }
    void Linklist::push_back(int value) {//尾插法
        LLNode* p = build(value);
        if(!p) return;
        if(!tail) {
            head = tail = p;
        } else {
            tail->next=p;
            tail=p;
        }
        length++;
    }
    bool Linklist::insert(int position, int value){
        if (position <= 0) {
            push_front(value);
            return true;
        }
        if (position >= length) {
            push_back(value);
            return true;
        }
        int i=0;
        LLNode* prev=head;
        while(prev && i < position-1) {
            prev = prev->next;
            i++;
        }
        if(!prev) return false;
        LLNode* p = build(value);
        if(!p) return false;
        p->next = prev->next;
        prev->next = p;
        length++;
        return true;
    }
    bool Linklist::erase(int position){
        if (position < 0 || position >= length || !head)
            return false;
        if (position == 0) {
            LLNode* q=head->next;
            std::free(head);
            head=q;
            if(!head) tail=nullptr;
            length--;
            return true;
        }
        int i=0;
        LLNode* prev=head;
        while(prev && prev->next && i < position-1) {
            prev=prev->next;
            i++;
        }
        if(!prev || !prev->next)
            return false;
        LLNode* cur = prev->next;
        prev->next = cur->next;
        if (cur==tail)
            tail=prev;
        std::free(cur);
        length--;
        return true;
    }
    int Linklist::get(int position) const {
        if(position < 0 || position >= length)
            return 0;
        int i = 0;
        LLNode* p = head;
        while(p && i < position) {
            p = p->next;
            i++;
        }
        return p ? p->value : 0;
    }


    }
