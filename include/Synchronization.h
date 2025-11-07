#ifndef SYNCHRONIZATION_H
#define SYNCHRONIZATION_H

#include <mutex>
#include <condition_variable>
#include <thread>
#include <vector>
#include <semaphore.h>

class Semaphore {
private:
    int count;
    std::mutex mtx;
    std::condition_variable cv;
    
public:
    Semaphore(int initial);
    void wait();
    void signal();
    int getCount() { return count; }
};

class ProducerConsumer {
private:
    static const int BUFFER_SIZE = 10;
    int buffer[BUFFER_SIZE];
    int in, out, count;
    Semaphore* empty;
    Semaphore* full;
    std::mutex mtx;
    bool running;
    
public:
    ProducerConsumer();
    ~ProducerConsumer();
    void produce(int item);
    int consume();
    void runSimulation(int numProducers, int numConsumers, int items);
    void stop() { running = false; }
};

class DiningPhilosophers {
private:
    static const int NUM_PHILOSOPHERS = 5;
    std::vector<std::mutex*> forks;
    bool running;
    
public:
    DiningPhilosophers();
    ~DiningPhilosophers();
    void philosopher(int id);
    void runSimulation();
    void stop() { running = false; }
};

#endif
