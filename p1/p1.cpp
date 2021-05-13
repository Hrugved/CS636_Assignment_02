#include <iostream>
#include <thread>
#include <mutex>
#include <atomic>
#include "LFqueue.h"

using namespace std;

LFqueue* agent = new LFqueue();
atomic<int> seatsLeft;
atomic<int> seatsDemanded;
mutex m_print; // via LFqueue.h

#define SEATS 80 // 80
#define CUSTOMERS 25 // 25
#define MAX_SEATS_PER_CUSTOMER 4 // 4

void _producer() {
    for(int i=1;i<=SEATS;i++) {
        agent->enqueue(i);
    }
}

void _consumer(int id) {
    int seatsToBook = (rand()%MAX_SEATS_PER_CUSTOMER)+1;
    seatsDemanded.fetch_add(seatsToBook);
    int seats[seatsToBook];
    int seatsBooked = 0;
    int seatReceived;
    while(seatsBooked<seatsToBook && seatsLeft.load()>0) {
        while(seatsBooked<seatsToBook) {
            if(agent->dequeue(seatReceived)) {
                seats[seatsBooked++]=seatReceived;
                seatsLeft.fetch_sub(1);
            } else break;
        }
    }
    string log = "Consumer with ID:"+to_string(id)+" has booked ";
    if(seatsBooked<seatsToBook) {
        log+=(to_string(seatsBooked)+" seats out of ");
    }
    log+=to_string(seatsToBook)+" seats";
    if(seatsBooked>0) {
        log+=" with seat numbers ";
        for (int j = 0; j < seatsBooked; ++j) {
            log+=("S"+to_string(seats[j]));
            if(j<(seatsBooked-1)) log+=", ";
        }
    }
    log+=".\n";
    {
        lock_guard<mutex> lock(m_print);
        cout<<log;
    }
}

int main(void){
    seatsLeft.store(SEATS);
    thread producer(_producer);
    thread consumers[CUSTOMERS];
    for(int i=0;i<CUSTOMERS;i++) {
        consumers[i]=thread(_consumer,i+1);
    }
    producer.join();
//    agent->dump();
    for(int i=0;i<CUSTOMERS;i++) {
        consumers[i].join();
    }
    {
        lock_guard<mutex> lock(m_print);
        cout<<"\nStats:\n";
        cout<<"\tSeats Available = "<<SEATS<<"\n";
        cout<<"\tSeats Demanded = "<<seatsDemanded.load()<<"\n";
        cout<<"\tSeats Booked = "<<(SEATS-seatsLeft.load())<<"\n";
    }
    return 0;
}
