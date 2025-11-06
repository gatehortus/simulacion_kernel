#ifndef DISK_SCHEDULER_H
#define DISK_SCHEDULER_H

#include <vector>
#include <string>
#include <algorithm>
#include <cmath>

struct DiskRequest {
    int cylinder;
    int arrival_time;
    int process_id;
};

struct DiskStats {
    int total_head_movement;
    double avg_wait_time;
    std::vector<int> sequence;
    std::string algorithm;
};

class DiskScheduler {
private:
    int num_cylinders;
    int current_head;
    std::vector<DiskRequest> request_queue;
    
public:
    DiskScheduler(int cylinders = 200, int initial_head = 50);
    
    void addRequest(int cylinder, int arrival_time, int pid);
    void clearQueue();
    
    // Algoritmos de planificación
    DiskStats scheduleFCFS();
    DiskStats scheduleSSTF();
    DiskStats scheduleSCAN(bool direction_up = true);
    DiskStats scheduleCSCAN();
    DiskStats scheduleLOOK(bool direction_up = true);
    
    // Utilidades
    void printStats(const DiskStats& stats);
    std::string visualizeDisk(const DiskStats& stats);
    void setHead(int position) { current_head = position; }
    int getHead() const { return current_head; }
    size_t getQueueSize() const { return request_queue.size(); }
};

#endif // DISK_SCHEDULER_H
