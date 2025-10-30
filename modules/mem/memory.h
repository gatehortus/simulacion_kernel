@'
#pragma once
#include <vector>
#include <queue>
#include <algorithm>
#include <iostream>
#include <map>

class MemoryManager {
private:
    int frames;
    std::vector<std::pair<int,int>> frame_table;
    std::queue<int> fifo_queue;
    std::map<int,int> access_time;
    int time_counter = 0;
    int faults = 0;
    int accesses = 0;
    std::vector<std::pair<int,int>> stats_rows;
public:
    MemoryManager(int f=4);
    void accessPage(int pid, int page);
    void showStatus();
    std::vector<std::pair<int,int>> getStatsRows(){ return stats_rows; }
};
'@ | Out-File -Encoding utf8 modules\mem\memory.h
