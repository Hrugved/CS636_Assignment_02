#include <mutex>
#include <shared_mutex>

struct node_t;
struct pointer_t;
struct queue_t;

using namespace std;

extern mutex m_print; // for convenience global multifile access to print

class LFqueue {
    queue_t* Q;
public:
    LFqueue();
    void enqueue(int);
    bool dequeue(int&);
    void dump();
};

