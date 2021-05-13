#include "hashtable.h"
#include <fstream>
#include <iostream>
#include <thread>
#include <mutex>

using namespace std;

const string delimiter = ",";

CHashTable hashtable;

mutex m_print;
void print(string& s) {
    lock_guard<mutex> lk(m_print);
    cout<<s;
}

void _reader(int id) {
    string name = "input"+to_string(id)+".txt";
    ifstream F(name);
    if (F.is_open()){
        string s;
        while(getline(F, s)){
            string log = s+": ";
            size_t pos = 0;
            string op,s1;
            pos = s.find(delimiter);
            op = s.substr(0, pos);
            s.erase(0, pos + delimiter.length());
            if(op=="I") {
                pos = s.find(delimiter);
                s1 = s.substr(0, pos);
                s.erase(0, pos + delimiter.length());
                if(!hashtable.insert(s1,s)) log+="ENOSPACE\n";
                else log+="SUCCESS\n";
            } else if(op=="R") {
                if(!hashtable.remove(s)) log+="ENOWORD\n";
                else log+="SUCCESS\n";
            } else { // op=="F"
                string out = hashtable.find(s);
                if(out=="") log+="ENOWORD\n";
                else log+=("\n"+out);
            }
            print(log);
        }
        F.close();
    }
}

int main(void) {
    thread readers[THREADS];
    for(int i=0;i<THREADS;i++) {
        readers[i]=thread(_reader,i+1);
    }
    for(int i=0;i<THREADS;i++) {
        readers[i].join();
    }
    cout<<"---Finished---"<<"\n";
    return 0;
}
