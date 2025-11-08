#ifndef MEMORY_H
#define MEMORY_H

#include <string>
#include <vector>
#include <queue>
#include <map>
#include <set>

class Page {
public:
    int pageNumber;
    int frameNumber;
    bool valid;
    int lastUsed;
    int loadTime;

    Page(int pn);
};

class MemoryManager {
protected: // Cambiado de 'private' a 'protected' para acceso en clases derivadas
    int numFrames;
    std::vector<int> frames;
    int currentTime;

public:
    int pageFaults;
    int pageHits;

    MemoryManager(int frames);
    virtual ~MemoryManager() {}
    virtual bool accessPage(int pageNumber) = 0;
    virtual std::string getName() = 0;
    void printStatistics();
    std::vector<int> getFrames() { return frames; }

protected:
    bool isPageInMemory(int pageNumber);
    int findFrame(int pageNumber);
};

class LRUMemory : public MemoryManager {
private:
    std::map<int, int> pageToFrame;
    std::map<int, int> lastUsed;

public:
    LRUMemory(int frames);
    bool accessPage(int pageNumber) override;
    std::string getName() override { return "LRU"; }
};

class FIFOMemory : public MemoryManager {
private:
    std::queue<int> fifoQueue;
    std::map<int, int> pageToFrame;

public:
    FIFOMemory(int frames);
    bool accessPage(int pageNumber) override;
    std::string getName() override { return "FIFO"; }
};

class WorkingSetMemory : public MemoryManager {
private:
    std::map<int, int> pageToFrame;
    std::map<int, int> lastUsed;
    int windowSize;

public:
    WorkingSetMemory(int frames, int window = 5);
    bool accessPage(int pageNumber) override;
    std::string getName() override { return "Working Set"; }
};

#endif
