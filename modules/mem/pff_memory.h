#ifndef PFF_MEMORY_H
#define PFF_MEMORY_H

#include <vector>
#include <unordered_set>
#include <queue>
#include <string>

struct PFFStats {
    int page_faults;
    int page_hits;
    double fault_rate;
    int frame_adjustments;
    std::vector<int> working_set_sizes;
    std::string algorithm;
};

class PFFMemoryManager {
private:
    int max_frames;
    int min_frames;
    int current_frames;
    double upper_threshold;  // Si PFF > upper, aumentar frames
    double lower_threshold;  // Si PFF < lower, reducir frames
    
    std::unordered_set<int> resident_pages;
    std::queue<int> page_queue;  // Para FIFO dentro del working set
    
    int page_faults;
    int page_hits;
    int frame_adjustments;
    int last_fault_time;
    int current_time;
    
    std::vector<int> working_set_history;
    
    void adjustFrames();
    double calculatePFF();
    
public:
    PFFMemoryManager(int max_frames = 10, int initial_frames = 5, 
                     double upper = 0.4, double lower = 0.1);
    
    bool accessPage(int page_number);
    void reset();
    PFFStats getStats() const;
    void printStats() const;
    std::string visualizeMemory() const;
    
    int getCurrentFrames() const { return current_frames; }
    int getPageFaults() const { return page_faults; }
    int getPageHits() const { return page_hits; }
};

// Working Set Algorithm
class WorkingSetMemoryManager {
private:
    int max_frames;
    int window_size;  // τ (tau) - tamaño de la ventana de tiempo
    
    struct PageEntry {
        int page_number;
        int last_access_time;
        bool reference_bit;
    };
    
    std::vector<PageEntry> memory;
    int current_time;
    int page_faults;
    int page_hits;
    
    std::vector<int> working_set_history;
    
    void updateWorkingSet();
    int findVictim();
    
public:
    WorkingSetMemoryManager(int max_frames = 10, int window = 5);
    
    bool accessPage(int page_number);
    void reset();
    PFFStats getStats() const;
    void printStats() const;
    std::string visualizeMemory() const;
    
    int getCurrentWorkingSetSize() const;
    int getPageFaults() const { return page_faults; }
    int getPageHits() const { return page_hits; }
};

#endif // PFF_MEMORY_H
