
#pragma once
#include <queue>
#include <iostream>

class IOManager {
private:
    std::queue<int> io_queue;
public:
    void requestIO(int pid) {
        io_queue.push(pid);
        std::cout << "IO: Request from pid="<<pid<<" queued\n";
    }
    void processNext() {
        if (io_queue.empty()) { std::cout << "IO: queue empty\n"; return; }
        int pid = io_queue.front(); io_queue.pop();
        std::cout << "IO: processed pid="<<pid<<"\n";
    }
};
