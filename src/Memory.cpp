#include "../include/Memory.h"
#include <iostream>
#include <algorithm>
#include <climits>

Page::Page(int pn) : pageNumber(pn), frameNumber(-1), valid(false), lastUsed(0), loadTime(0) {}

MemoryManager::MemoryManager(int f) : numFrames(f), currentTime(0), pageFaults(0), pageHits(0) {
    frames.resize(numFrames, -1);
}

bool MemoryManager::isPageInMemory(int pageNumber) {
    return std::find(frames.begin(), frames.end(), pageNumber) != frames.end();
}

int MemoryManager::findFrame(int pageNumber) {
    for (int i = 0; i < numFrames; i++) {
        if (frames[i] == pageNumber) return i;
    }
    return -1;
}

void MemoryManager::printStatistics() {
    int total = pageFaults + pageHits;
    std::cout << "\n=== Estadisticas de Memoria - " << getName() << " ===" << std::endl;
    std::cout << "Page Faults: " << pageFaults << std::endl;
    std::cout << "Page Hits: " << pageHits << std::endl;
    std::cout << "Total accesos: " << total << std::endl;
    if (total > 0) {
        std::cout << "Tasa de fallos: " << (pageFaults * 100.0 / total) << "%" << std::endl;
        std::cout << "Tasa de aciertos: " << (pageHits * 100.0 / total) << "%" << std::endl;
    }
}

// LRU Implementation
LRUMemory::LRUMemory(int f) : MemoryManager(f) {}

bool LRUMemory::accessPage(int pageNumber) {
    currentTime++;
    
    if (pageToFrame.find(pageNumber) != pageToFrame.end()) {
        // Page hit
        pageHits++;
        lastUsed[pageNumber] = currentTime;
        return true;
    }
    
    // Page fault
    pageFaults++;
    
    if (pageToFrame.size() < numFrames) {
        // Hay marcos libres
        int frameIndex = pageToFrame.size();
        pageToFrame[pageNumber] = frameIndex;
        frames[frameIndex] = pageNumber;
        lastUsed[pageNumber] = currentTime;
    } else {
        // Reemplazar pagina menos recientemente usada
        int lruPage = -1;
        int lruTime = INT_MAX;
        
        for (auto& pair : lastUsed) {
            if (pair.second < lruTime) {
                lruTime = pair.second;
                lruPage = pair.first;
            }
        }
        
        int frameIndex = pageToFrame[lruPage];
        pageToFrame.erase(lruPage);
        lastUsed.erase(lruPage);
        
        pageToFrame[pageNumber] = frameIndex;
        frames[frameIndex] = pageNumber;
        lastUsed[pageNumber] = currentTime;
    }
    
    return false;
}

// FIFO Implementation
FIFOMemory::FIFOMemory(int f) : MemoryManager(f) {}

bool FIFOMemory::accessPage(int pageNumber) {
    currentTime++;
    
    if (pageToFrame.find(pageNumber) != pageToFrame.end()) {
        // Page hit
        pageHits++;
        return true;
    }
    
    // Page fault
    pageFaults++;
    
    if (pageToFrame.size() < numFrames) {
        // Hay marcos libres
        int frameIndex = pageToFrame.size();
        pageToFrame[pageNumber] = frameIndex;
        frames[frameIndex] = pageNumber;
        fifoQueue.push(pageNumber);
    } else {
        // Reemplazar primera pagina (FIFO)
        int oldPage = fifoQueue.front();
        fifoQueue.pop();
        
        int frameIndex = pageToFrame[oldPage];
        pageToFrame.erase(oldPage);
        
        pageToFrame[pageNumber] = frameIndex;
        frames[frameIndex] = pageNumber;
        fifoQueue.push(pageNumber);
    }
    
    return false;
}

// Working Set Implementation
WorkingSetMemory::WorkingSetMemory(int f, int window) 
    : MemoryManager(f), windowSize(window) {}

bool WorkingSetMemory::accessPage(int pageNumber) {
    currentTime++;
    
    if (pageToFrame.find(pageNumber) != pageToFrame.end()) {
        // Page hit
        pageHits++;
        lastUsed[pageNumber] = currentTime;
        return true;
    }
    
    // Page fault
    pageFaults++;
    
    // Eliminar paginas fuera del working set
    std::vector<int> toRemove;
    for (auto& pair : lastUsed) {
        if (currentTime - pair.second > windowSize) {
            toRemove.push_back(pair.first);
        }
    }
    
    for (int page : toRemove) {
        int frameIndex = pageToFrame[page];
        pageToFrame.erase(page);
        lastUsed.erase(page);
        frames[frameIndex] = -1;
    }
    
    if (pageToFrame.size() < numFrames) {
        // Buscar marco libre
        int frameIndex = -1;
        for (int i = 0; i < numFrames; i++) {
            if (frames[i] == -1) {
                frameIndex = i;
                break;
            }
        }
        
        if (frameIndex == -1) {
            frameIndex = pageToFrame.size();
        }
        
        pageToFrame[pageNumber] = frameIndex;
        frames[frameIndex] = pageNumber;
        lastUsed[pageNumber] = currentTime;
    } else {
        // Reemplazar pagina menos recientemente usada
        int lruPage = -1;
        int lruTime = INT_MAX;
        
        for (auto& pair : lastUsed) {
            if (pair.second < lruTime) {
                lruTime = pair.second;
                lruPage = pair.first;
            }
        }
        
        int frameIndex = pageToFrame[lruPage];
        pageToFrame.erase(lruPage);
        lastUsed.erase(lruPage);
        
        pageToFrame[pageNumber] = frameIndex;
        frames[frameIndex] = pageNumber;
        lastUsed[pageNumber] = currentTime;
    }
    
    return false;
}
