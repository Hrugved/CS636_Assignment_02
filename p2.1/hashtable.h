#define SIZE 1 << 12
#define MAX_KEY 1 << 5
#define MAX_VALUE 1 << 8
#define THREADS 4

#include <string>
#include <shared_mutex>

using namespace std;

struct Node {
    string word;
    string sentence;
    Node* next;
    Node(string _word="",string _sentence="",Node* _next=nullptr): word(_word), sentence(_sentence) ,next(_next) {}
};

class Bucket {
public:
    Node* head;
    shared_timed_mutex mtx;
    Bucket() {head= new Node();}
};

class CHashTable {
    Bucket* map[SIZE];
    unsigned long hash(string& str);
public:
    CHashTable();
    string find(string word);
    bool insert(string word, string sentence);
    bool remove(string word);
};

