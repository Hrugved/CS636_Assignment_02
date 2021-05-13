#include "hashtable.h"
#include<atomic>
#define CAS atomic_compare_exchange_strong

unsigned long CHashTable::hash(string& str) {
    unsigned long hash = 5381;
    for(int c : str)
        hash = ((hash << 5) + hash) + c; /* hash * 33 + c */
    return hash;
}

CHashTable::CHashTable() {}

bool CHashTable::insert(string word, string sentence) {
    if(sentence.size()>MAX_VALUE) {
        return false;
    }
    Node* node = new Node(word,sentence);
    unsigned int key = hash(word)%SIZE;
    Bucket(&map[key]).insert(node);
    return true;
}

bool CHashTable::remove(string word){
    unsigned int key = hash(word)%SIZE;
    Bucket b = Bucket(&map[key]);
    if(!b.remove(word)) return false;
    while(true) {
        if(!b.remove(word)) break;
    }
    return true;
}

string CHashTable::find(string word) {
    string s="";
    unsigned int key = hash(word)%SIZE;
    Node* p = map[key].m.load().next;
    while(p!= nullptr) {
        if(word==p->word) s+="\t * "+p->sentence+"\n";
        p = p->m.load().next;
    }
    return s;
}

bool Bucket::search(string word) {
    try_again:
    prev = head;
    t1 = prev->m.load();
    while(true) {
        if(t1.next==nullptr) return false; // empty
        t2 = ptr(t1.next)->m;
        string cword = ptr(t1.next)->word;
        Meta t3(0,t1.next,t1.tag);
        if(ptr(prev)->m.load() != t3) goto try_again;
        if (!isSet(t2.next)) {
            if(cword>=word) return cword==word; // found node with >= key
            prev = t1.next; // keep iterating
        } else {
            // physical delete
            if(CAS(&prev->m,&t3,Meta(0,t2.next,t1.tag+1))) {
                delete(ptr(t1.next));
                t2.tag = t1.tag+1;
            } else goto try_again;
        }
        t1=t2;
    }
}

bool Bucket::insert(Node* node) {
    string word = node->word;
    while(true) {
        search(word);
        node->m = Meta(false,t1.next,node->m.load().tag);
        if(CAS(&prev->m,new Meta(false,t1.next,t1.tag),Meta(false,node,t1.tag+1))) return true;
    }
}

bool Bucket::remove(string word) {
    while(true) {
        if(!search(word)) return false;
        // logical delete
        if(!CAS(&ptr(t1.next)->m,new Meta(false,t2.next,t2.tag),Meta(true,t2.next,t2.tag+1))) continue;
        if(CAS(&prev->m,new Meta(false,t1.next,t1.tag),Meta(false,t2.next,t1.tag+1))) {
            delete(ptr(t1.next));
        }
        else search(word);
        return true;
    }
}
