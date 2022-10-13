#pragma once

#include <stdio.h>

#include <atomic>
#include <iostream>
#include <list>
#include <string>

template <typename T>
class Node {
   public:
    T data;
    Node<T> *next;
};

template <typename T>
class LockFreeQueue {
   public:
    LockFreeQueue();
    //入队
    void push_back(T t);
    //出队
    T pop_front(void);
    //判队空
    bool isEmpty(void);

   private:
    Node<T> *head_;
    Node<T> *tail_;
    std::atomic_int elementNumbers_;
};

template <typename T>
LockFreeQueue<T>::LockFreeQueue()
    : head_(NULL), tail_(new Node<T>), elementNumbers_(0) {
    // 初始化一个dummy结点
    head_ = tail_;
    tail_->next = NULL;
}

// 有个潜在的问题，
// 如果T1线程在用CAS更新指针前，线程停掉了或者挂掉了。其他线程就死循环了。
// template <typename T>
// void LockFreeQueue<T>::push_back(T t) {
//     auto newVal = new Node<T>;
//     newVal->data = t;
//     newVal->next = NULL;

//     Node<T> *p;
//     do {
//         //此操作暂时可删除，之后会跟新
//         p = tail_;  // 取链表尾指针的快照
//         如果没有把结点连在为指针上,则重新再试
//     } while (!__sync_bool_compare_and_swap(&p->next, NULL, newVal));

//     // 移动tail_
//     /**
//      * @brief 为什么此处不需要判断是否成功？
//      * 因为如果T1它的while
//      * cas时成功的，p->next被赋值成newVal了，那么其他随后线程的CAS都会失败，继续循环
//      * 此时，T1线程还没有更新tail指针，那么其他线程继续失败，因为p->next不是NULL了
//      * 直到T1线程更新完tail指针，其他线程的某个线程才后往下走。
//      * 所以，只要一个线程从while中走出来，那么就意味着它已经独占了，tail指针必然被更新。
//      */
//     __sync_bool_compare_and_swap(&tail_, p, newVal);
//     elementNumbers_++;
// }

template <typename T>
void LockFreeQueue<T>::push_back(T t) {
    auto newVal = new Node<T>;
    newVal->data = t;
    newVal->next = NULL;

    Node<T> *cur_tail;
    while (true) {
        cur_tail = tail_;  // 获取当前的尾指针
        auto next = tail_->next;

        // 如果此时的尾指针已经被移动
        if (cur_tail != tail_) continue;
        // 如果尾指针的next的next不为NULL，所以队列的尾部被添加了元素，但tail_没有移动到队尾
        // 此时就需要将tail_移动到队尾
        if (next != NULL) {
            __sync_bool_compare_and_swap(&tail_, cur_tail, next);
            continue;
        }

        if (__sync_bool_compare_and_swap(&cur_tail->next, NULL, newVal)) break;
    }
    __sync_bool_compare_and_swap(&tail_, cur_tail, newVal);
}

// 有一个问题 ？？
// 当head、tail指向dummy,还没有数
// 这时候 push_back加入结点A，但tail还没更新
// 这时候 pop_front，head和tail就不一致了 tail指向dummy，head指向A
// template <typename T>
// T LockFreeQueue<T>::pop_front() {
//     Node<T> *p;

//     do {
//         //获取第一个节点快照
//         p = head_->next;
//         if (!p) {
//             return 0;
//         }
//     } while (!__sync_bool_compare_and_swap(&head_->next, p, p->next));
//     elementNumbers_--;
//     return p->data;
// }

template <typename T>
T LockFreeQueue<T>::pop_front() {
    T value;
    Node<T> *cur_head, *cur_tail, *next;
    while (true) {
        // 取出头指针，尾指针，和第一个元素的指针
        cur_head = head_;
        cur_tail = tail_;
        next = head_->next;

        // q->head 指针已移动，重新取head指针
        if (cur_head != head_) continue;
        // 如果是空队列
        if (cur_head == cur_tail && next == NULL) return 0;
        // 如果tail落后了
        if (cur_head == cur_tail && next != NULL) {
            __sync_bool_compare_and_swap(&tail_, cur_tail, next);
            continue;
        }
        // 移动head指针成功后，取出数据
        if (__sync_bool_compare_and_swap(&head_, cur_head, next)) {
            value = next->data;
            break;
        }
    }
    delete cur_head;
    return value;
}

template <typename T>
bool LockFreeQueue<T>::isEmpty() {
    if (elementNumbers_ == 0) {
        return true;
    } else {
        return false;
    }
}