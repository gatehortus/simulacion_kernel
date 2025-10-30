
#pragma once
#include <mutex>
#include <condition_variable>
#include <queue>
#include <iostream>

class SyncManager {
private:
    std::mutex mtx;
    std::condition_variable cv;
    std::queue<int> buffer;
    const unsigned int MAX_BUFFER = 5;
    int produced_count = 0;
    int consumed_count = 0;
public:
    void produce();
    void consume();
    void printStatus();
};
