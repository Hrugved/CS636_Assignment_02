#include "hashtable.h"

unsigned long CHashTable::hash(string& str) {
    unsigned long hash = 5381;
    for(int c : str)
        hash = ((hash << 5) + hash) + c; /* hash * 33 + c */
    return hash;
}

CHashTable::CHashTable() {}

string CHashTable::find(string word) {
    unsigned long key = hash(word)%SIZE;
    if(map[key]==nullptr) return "";
    Node* head = map[key]->head;
    shared_lock<shared_timed_mutex> lk(map[key]->mtx);
    Node* t = head->next;
    string s;
    while(t!= nullptr) {
        if(t->word==word) s+="\t * "+t->sentence+"\n";
        t=t->next;
    }
    return s;
}

bool CHashTable::insert(string word, string sentence) {
    if(sentence.size()>MAX_VALUE) {
        return false;
    }
    unsigned int key = hash(word)%SIZE;
    if(map[key]== nullptr) {
        map[key] = new Bucket();
    }
    Node* node = new Node(word,sentence);
    Node* head = map[key]->head;
    unique_lock<shared_timed_mutex> lk(map[key]->mtx);
    node->next = head->next;
    head->next = node;
    return true;
}

bool CHashTable::remove(string word) {
    unsigned int key = hash(word)%SIZE;
    if(map[key]==nullptr) return false;
    Node* head = map[key]->head;
    Node* prev = head;
    unique_lock<shared_timed_mutex> lk(map[key]->mtx);
    Node* curr = head->next;
    int count=0;
    while (curr!=nullptr) {
        if(curr->word==word) {
            Node* del = curr;
            prev->next=curr->next;
            curr=curr->next;
            delete del;
            count++;
        } else {
            prev = curr;
            curr = curr->next;
        }
    }
    return count!=0;
}
