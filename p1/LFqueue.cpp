#include <atomic>
#include "LFqueue.h"
#include <iostream>

using namespace std;
#define CAS atomic_compare_exchange_strong

shared_timed_mutex mtx;

// 16-byte struct which can be operated atomically
struct pointer_t {
    node_t* ptr = nullptr;
    unsigned int count = 0;
    pointer_t(node_t* aPtr,unsigned int aCount)noexcept : ptr(aPtr), count(aCount) {}
    pointer_t() noexcept{}
    bool operator==(const pointer_t& o){
        return this->ptr == o.ptr && this->count == o.count;
    }
    pointer_t& operator=(pointer_t* p){
        ptr = p->ptr;
        count = p->count;
        return *this;
    }
};

struct node_t {
    int val=-1;
    atomic<pointer_t> next;
    node_t(int aVal, pointer_t aNext) : val(aVal), next(aNext) {}
    node_t(){}
};

struct queue_t {
    atomic<pointer_t> head;
    atomic<pointer_t> tail;
};

LFqueue::LFqueue() {
    node_t* node = new node_t();
    pointer_t ptr = pointer_t(node,0);
    Q = new queue_t();
    Q->head.store(ptr);
    Q->tail.store(ptr);
}

void LFqueue::enqueue(int val) {
    shared_lock<shared_timed_mutex> lk(mtx); // Lock will only block when dump() is called by user
    node_t *node = new node_t(val, pointer_t());
    pointer_t tail,next;
    while(true) {
        tail = Q->tail.load();
        next = tail.ptr->next.load();
        if(tail==Q->tail.load()) {
            if(next.ptr==nullptr) {
                if(CAS(&tail.ptr->next,&next,pointer_t(node,next.count+1))) break;
            } else {
                CAS(&Q->tail,&tail,pointer_t(next.ptr,tail.count+1));
            }
        }
    }
    CAS(&Q->tail,&tail,pointer_t(node,tail.count+1));
}

bool LFqueue::dequeue(int& val) {
    shared_lock<shared_timed_mutex> lk(mtx); // Lock will only block when dump() is called by user
    pointer_t head,tail,next;
    while(true) {
        head = Q->head.load();
        tail = Q->tail.load();
        next = head.ptr->next.load();
        if(head==Q->head.load()) {
            if(head.ptr==tail.ptr) {
                if(next.ptr== nullptr) return false; // empty
                CAS(&Q->tail,&tail,pointer_t(next.ptr,tail.count+1));
            } else {
                val = next.ptr->val; // value
                if(CAS(&Q->head,&head,pointer_t(next.ptr,head.count+1))) break; // done
            }
        }
    }
    delete head.ptr;
    return true;
}

void LFqueue::dump() {
    unique_lock<shared_timed_mutex> lk(mtx);
    pointer_t head,tail,next,curr;
    head = Q->head.load();
    curr = head.ptr->next;
    string s = "Queue: ";
    if(curr.ptr==nullptr) {
        s+="EMPTY";
    } else {
        s+="\n\t";
        while(curr.ptr!=nullptr) {
            s+=(to_string(curr.ptr->val)+" -> ");
            curr = curr.ptr->next;
        }
        s+="NULL";
    }
    s+="\n";
    {
        lock_guard<mutex> lock(m_print);
        cout<<s;
    }
}