#ifndef HEAP_ALLOCATOR_H
#define HEAP_ALLOCATOR_H

#include <vector>
#include <map>
#include <string>
#include <cmath>

struct Block {
    size_t size;
    bool is_free;
    size_t address;
    int level;  // Para Buddy System
};

struct HeapStats {
    size_t total_memory;
    size_t used_memory;
    size_t free_memory;
    double fragmentation_external;
    double fragmentation_internal;
    int allocations;
    int deallocations;
    int coalesces;
    std::string algorithm;
};

// Buddy System Allocator
class BuddyAllocator {
private:
    size_t total_size;
    int max_level;
    std::vector<std::vector<Block*>> free_lists;
    std::map<size_t, Block*> allocated_blocks;
    
    HeapStats stats;
    
    int calculateLevel(size_t size);
    size_t getLevelSize(int level);
    Block* split(Block* block, int target_level);
    void coalesce(Block* block);
    Block* findBuddy(Block* block);
    void updateFragmentation();
    
public:
    BuddyAllocator(size_t size = 1024);
    ~BuddyAllocator();
    
    void* allocate(size_t size);
    void deallocate(void* ptr);
    void reset();
    
    HeapStats getStats() const { return stats; }
    void printStats() const;
    std::string visualizeHeap() const;
};

// Segregated Free List Allocator
class SegregatedAllocator {
private:
    struct SizeClass {
        size_t block_size;
        std::vector<Block*> free_blocks;
        size_t total_blocks;
    };
    
    std::vector<SizeClass> size_classes;
    std::map<size_t, Block*> allocated_blocks;
    size_t total_size;
    
    HeapStats stats;
    
    int findSizeClass(size_t size);
    Block* allocateNewSlab(int class_index);
    void updateFragmentation();
    
public:
    SegregatedAllocator(size_t size = 1024);
    ~SegregatedAllocator();
    
    void* allocate(size_t size);
    void deallocate(void* ptr);
    void reset();
    
    HeapStats getStats() const { return stats; }
    void printStats() const;
    std::string visualizeHeap() const;
};

#endif // HEAP_ALLOCATOR_H
