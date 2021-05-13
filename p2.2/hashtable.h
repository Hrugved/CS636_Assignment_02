#define SIZE 1 << 12
#define MAX_KEY 1 << 5
#define MAX_VALUE 1 << 8
#define THREADS 4

#include<iostream>
#include <string>
#include<atomic>

using namespace std;

struct Node;
struct Meta; // 16 byte object for atomic operations
struct CHashTable; // main data struture
struct Bucket; // auxillary struct for list specific operations

///  Pointer(MSB bit is set/unset) specific functions  ///
const uintptr_t MASK = ((1ULL << 63) - 1);
const uintptr_t setBit = (uintptr_t) 1 << 63;

struct Node;

inline Node* ptr(Node *p) {
    return reinterpret_cast<Node *>(((intptr_t) p << 16) >> 16);
}

inline bool isSet(Node *p) {
    return ((uintptr_t) p >> 63 & 1) == 1;
}

inline void set(Node *p) {
    p = reinterpret_cast<Node *>(((uintptr_t) p & MASK) | (setBit));
}

inline void unset(Node *p) {
    p = reinterpret_cast<Node *>(((uintptr_t) p & MASK));
}

inline Node *getSet(Node *p) {
    return reinterpret_cast<Node *>(((uintptr_t) p & MASK) | (setBit));
}

inline Node *getUnset(Node *p) {
    return reinterpret_cast<Node *>(((uintptr_t) p & MASK));
}
///////////////

struct Meta {
    Node* next; // MSB = marking bit, points to next Node
    unsigned long tag; // for preventing ABA problem
    Meta(Node* _next=nullptr,unsigned long _tag=0)noexcept : next(_next), tag(_tag) {}
    Meta(bool mark,Node* _next,unsigned long _tag)noexcept : next(_next), tag(_tag) {
        if(mark) set(next);
        else unset(next);
    }
    bool operator!=(const Meta& rhs){
        return (next!=rhs.next || tag!=rhs.tag);
    }
    bool operator == (const Meta &rhs) const {
        return (next==rhs.next && tag==rhs.tag);
    }
};

struct Node {
    string word; //key
    string sentence;
    atomic<Meta> m;
    Node(string _word="", string _sentence="", Meta _m = Meta()) : word(_word), sentence(_sentence), m(_m) {}
};

struct CHashTable {
    Node map[SIZE];
    unsigned long hash(string& str);
    CHashTable();
    bool insert(string word, string sentence);
    bool remove(string word);
    string find(string word);
};

struct Bucket {
    Node *prev;
    Meta t1;
    Meta t2;
    Node* head;
    Bucket(Node* _head) : head(_head) {}
    bool insert(Node* node);
    bool remove(string word);
    bool search(string key);
};

