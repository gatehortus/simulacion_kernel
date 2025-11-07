#include "../include/Synchronization.h"
#include <iostream>
#include <chrono>
#include <thread>

// Semaphore Implementation
Semaphore::Semaphore(int initial) : count(initial) {}

void Semaphore::wait() {
    std::unique_lock<std::mutex> lock(mtx);
    while (count <= 0) {
        cv.wait(lock);
    }
    count--;
}

void Semaphore::signal() {
    std::unique_lock<std::mutex> lock(mtx);
    count++;
    cv.notify_one();
}

// Producer-Consumer Implementation
ProducerConsumer::ProducerConsumer() 
    : in(0), out(0), count(0), running(true) {
    empty = new Semaphore(BUFFER_SIZE);
    full = new Semaphore(0);
}

ProducerConsumer::~ProducerConsumer() {
    delete empty;
    delete full;
}

void ProducerConsumer::produce(int item) {
    empty->wait();
    
    mtx.lock();
    buffer[in] = item;
    in = (in + 1) % BUFFER_SIZE;
    count++;
    std::cout << "Producido: " << item << " (Buffer: " << count << "/" << BUFFER_SIZE << ")" << std::endl;
    mtx.unlock();
    
    full->signal();
}

int ProducerConsumer::consume() {
    full->wait();
    
    mtx.lock();
    int item = buffer[out];
    out = (out + 1) % BUFFER_SIZE;
    count--;
    std::cout << "Consumido: " << item << " (Buffer: " << count << "/" << BUFFER_SIZE << ")" << std::endl;
    mtx.unlock();
    
    empty->signal();
    return item;
}

void ProducerConsumer::runSimulation(int numProducers, int numConsumers, int items) {
    std::cout << "\n=== Simulacion Productor-Consumidor ===" << std::endl;
    std::cout << "Productores: " << numProducers << ", Consumidores: " << numConsumers << std::endl;
    
    std::vector<std::thread> threads;
    
    // Crear productores
    for (int i = 0; i < numProducers; i++) {
        threads.push_back(std::thread([this, i, items]() {
            for (int j = 0; j < items; j++) {
                if (!running) break;
                int item = i * 100 + j;
                produce(item);
                std::this_thread::sleep_for(std::chrono::milliseconds(100));
            }
        }));
    }
    
    // Crear consumidores
    for (int i = 0; i < numConsumers; i++) {
        threads.push_back(std::thread([this, items, numProducers]() {
            int totalItems = items * numProducers / 2;
            for (int j = 0; j < totalItems; j++) {
                if (!running) break;
                consume();
                std::this_thread::sleep_for(std::chrono::milliseconds(150));
            }
        }));
    }
    
    // Esperar a que terminen
    for (auto& t : threads) {
        if (t.joinable()) t.join();
    }
    
    std::cout << "Simulacion completada" << std::endl;
}

// Dining Philosophers Implementation
DiningPhilosophers::DiningPhilosophers() : running(true) {
    for (int i = 0; i < NUM_PHILOSOPHERS; i++) {
        forks.push_back(new std::mutex());
    }
}

DiningPhilosophers::~DiningPhilosophers() {
    for (auto fork : forks) {
        delete fork;
    }
}

void DiningPhilosophers::philosopher(int id) {
    int leftFork = id;
    int rightFork = (id + 1) % NUM_PHILOSOPHERS;
    
    // Solucion: filosofo impar toma primero tenedor derecho
    if (id % 2 == 1) {
        std::swap(leftFork, rightFork);
    }
    
    for (int i = 0; i < 3 && running; i++) {
        // Pensar
        std::cout << "Filosofo " << id << " esta pensando" << std::endl;
        std::this_thread::sleep_for(std::chrono::milliseconds(200));
        
        // Tomar tenedores
        forks[leftFork]->lock();
        std::cout << "Filosofo " << id << " tomo tenedor izquierdo" << std::endl;
        
        forks[rightFork]->lock();
        std::cout << "Filosofo " << id << " tomo tenedor derecho" << std::endl;
        
        // Comer
        std::cout << "Filosofo " << id << " esta COMIENDO" << std::endl;
        std::this_thread::sleep_for(std::chrono::milliseconds(300));
        
        // Soltar tenedores
        forks[rightFork]->unlock();
        forks[leftFork]->unlock();
        std::cout << "Filosofo " << id << " solto los tenedores" << std::endl;
    }
    
    std::cout << "Filosofo " << id << " termino" << std::endl;
}

void DiningPhilosophers::runSimulation() {
    std::cout << "\n=== Simulacion Cena de los Filosofos ===" << std::endl;
    
    std::vector<std::thread> philosophers;
    
    for (int i = 0; i < NUM_PHILOSOPHERS; i++) {
        philosophers.push_back(std::thread(&DiningPhilosophers::philosopher, this, i));
    }
    
    for (auto& p : philosophers) {
        if (p.joinable()) p.join();
    }
    
    std::cout << "Simulacion completada sin deadlock" << std::endl;
}
