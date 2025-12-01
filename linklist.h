//
// Created by xiang on 25-10-8.
//
#ifndef LINKLIST_H
#define LINKLIST_H

#include <cstdlib>

namespace ds {

    struct LLNode {
        int value;
        LLNode *next;
    };

    class Linklist {
        LLNode *head;
        LLNode *tail;
        int length;

        // 新建一个节点
        LLNode* build(int v) {
            LLNode* p = static_cast<LLNode*>(std::malloc(sizeof(LLNode)));
            if (!p) return nullptr;
            p->value = v;
            p->next  = nullptr;
            return p;
        }

    public:
        Linklist() : head(nullptr), tail(nullptr), length(0) {}

        ~Linklist() {
            clear();
        }

        // 长度
        int size() const {
            return length;
        }

        // 只读头指针
        LLNode* gethead() const {
            return head;
        }

        // 清空链表
        void clear() {
            LLNode* p = head;
            while (p) {
                LLNode* q = p->next;
                std::free(p);
                p = q;
            }
            head = tail = nullptr;
            length = 0;
        }

        // 头插
        void push_front(int value) {
            LLNode* p = build(value);
            if (!p) return;
            p->next = head;
            head = p;
            if (!tail) tail = p;
            ++length;
        }

        // 尾插
        void push_back(int value) {
            LLNode* p = build(value);
            if (!p) return;
            if (!tail) {
                head = tail = p;
            } else {
                tail->next = p;
                tail = p;
            }
            ++length;
        }

        // 在 position 位置插入（0 <= position <= length）
        // position <= 0 视为头插；position >= length 视为尾插
        bool insert(int position, int value) {
            if (position <= 0) {
                push_front(value);
                return true;
            }
            if (position >= length) {
                push_back(value);
                return true;
            }

            int i = 0;
            LLNode* prev = head;
            while (prev && i < position - 1) {
                prev = prev->next;
                ++i;
            }
            if (!prev) return false;

            LLNode* p = build(value);
            if (!p) return false;

            p->next = prev->next;
            prev->next = p;
            ++length;
            return true;
        }

        // 删除 position 位置结点（0..length-1）
        bool erase(int position) {
            if (position < 0 || position >= length || !head) return false;
            if (position == 0) {
                LLNode* q = head->next;
                std::free(head);
                head = q;
                if (!head) tail = nullptr;
                --length;
                return true;
            }

            int i = 0;
            LLNode* prev = head;
            while (prev && prev->next && i < position - 1) { prev = prev->next; ++i; }
            if (!prev || !prev->next) return false;

            LLNode* cur = prev->next;
            prev->next = cur->next;
            if (cur == tail) tail = prev;
            std::free(cur);
            --length;
            return true;
        }
        // 获取 position 位置的值，不合法返回 0
        int get(int position) const {
            if (position < 0 || position >= length) return 0;
            int i = 0;
            LLNode* p = head;
            while (p && i < position) { p = p->next; ++i; }
            return p ? p->value : 0;
        }
    };
} // namespace ds

#endif // LINKLIST_H
