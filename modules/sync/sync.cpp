
#include "sync.h"
#include <thread>
#include <chrono>

void SyncManager::produce() {
    std::unique_lock<std::mutex> lock(mtx);
    if (buffer.size() >= MAX_BUFFER) {
        std::cout << "Buffer lleno. No se puede producir.\n";
        return;
    }
    int val = ++produced_count;
    buffer.push(val);
    std::cout << "Producido: "<<val<<" BufferSize="<<buffer.size()<<"\n";
    lock.unlock();
    cv.notify_all();
}

void SyncManager::consume() {
    std::unique_lock<std::mutex> lock(mtx);
    if (buffer.empty()) {
        std::cout << "Buffer vacío. Nada que consumir.\n";
        return;
    }
    int val = buffer.front(); buffer.pop();
    ++consumed_count;
    std::cout << "Consumido: "<<val<<" BufferSize="<<buffer.size()<<"\n";
    lock.unlock();
    cv.notify_all();
}

void SyncManager::printStatus() {
    std::unique_lock<std::mutex> lock(mtx);
    std::cout << "Buffer size="<<buffer.size()<<" Produced="<<produced_count<<" Consumed="<<consumed_count<<"\n";
}
