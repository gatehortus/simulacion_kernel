
#include "memory.h"
#include <iostream>
#include <algorithm>

MemoryManager::MemoryManager(int f): frames(f) {
    frame_table.assign(frames, std::make_pair(-1,-1));
    for (int i=0;i<frames;i++) fifo_queue.push(i);
}

void MemoryManager::accessPage(int pid, int page) {
    accesses++; time_counter++;
    for (int i=0;i<frames;i++) {
        if (frame_table[i].first==pid && frame_table[i].second==page) {
            access_time[i]=time_counter;
            std::cout << "Mem: HIT pid="<<pid<<" page="<<page<<"\n";
            return;
        }
    }
    faults++;
    int frame = fifo_queue.front(); fifo_queue.pop();
    std::pair<int,int> old = frame_table[frame];
    frame_table[frame] = std::make_pair(pid,page);
    fifo_queue.push(frame);
    access_time[frame]=time_counter;
    std::cout << "Mem: MISS pid="<<pid<<" page="<<page<<" (replaced frame="<<frame<<")\n";
    if (stats_rows.empty() || stats_rows.back().first!=frames) stats_rows.push_back({frames,faults});
    else stats_rows.back().second = faults;
}

void MemoryManager::showStatus() {
    std::cout << "Memory frames (frame -> pid:page):\n";
    for (int i=0;i<frames;i++) {
        auto pr = frame_table[i];
        if (pr.first==-1) std::cout << "  ["<<i<<"] empty\n";
        else std::cout << "  ["<<i<<"] "<<pr.first<<":"<<pr.second<<"\n";
    }
    double hit_rate = accesses? (1.0 - (double)faults / accesses) : 0.0;
    std::cout << "Accesses="<<accesses<<" Faults="<<faults<<" HitRate="<<hit_rate<<"\n";
}

