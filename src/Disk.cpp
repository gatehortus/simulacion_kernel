#include "../include/Disk.h"
#include <iostream>
#include <algorithm>
#include <cmath>
#include <climits>

DiskScheduler::DiskScheduler(int initial) 
    : currentPosition(initial), totalMovement(0) {}

void DiskScheduler::addRequest(int cylinder) {
    requests.push_back(cylinder);
}

void DiskScheduler::setRequests(std::vector<int> req) {
    requests = req;
}

void DiskScheduler::printResults() {
    std::cout << "\n=== Algoritmo: " << getName() << " ===" << std::endl;
    std::cout << "Secuencia de acceso: ";
    for (int cyl : sequence) {
        std::cout << cyl << " ";
    }
    std::cout << "\nMovimiento total del cabezal: " << totalMovement << " cilindros" << std::endl;
}

// FCFS Implementation
FCFSDisk::FCFSDisk(int initial) : DiskScheduler(initial) {}

void FCFSDisk::schedule() {
    totalMovement = 0;
    sequence.clear();
    
    int current = currentPosition;
    sequence.push_back(current);
    
    for (int request : requests) {
        totalMovement += abs(request - current);
        current = request;
        sequence.push_back(current);
    }
}

// SSTF Implementation
SSTFDisk::SSTFDisk(int initial) : DiskScheduler(initial) {}

void SSTFDisk::schedule() {
    totalMovement = 0;
    sequence.clear();
    
    std::vector<int> pending = requests;
    int current = currentPosition;
    sequence.push_back(current);
    
    while (!pending.empty()) {
        // Encontrar cilindro mas cercano
        int nearest = -1;
        int minDistance = INT_MAX;
        
        for (size_t i = 0; i < pending.size(); i++) {
            int distance = abs(pending[i] - current);
            if (distance < minDistance) {
                minDistance = distance;
                nearest = i;
            }
        }
        
        current = pending[nearest];
        totalMovement += minDistance;
        sequence.push_back(current);
        pending.erase(pending.begin() + nearest);
    }
}

// SCAN Implementation
SCANDisk::SCANDisk(int initial, int max) 
    : DiskScheduler(initial), movingUp(true), maxCylinder(max) {}

void SCANDisk::schedule() {
    totalMovement = 0;
    sequence.clear();
    
    std::vector<int> left, right;
    
    for (int req : requests) {
        if (req < currentPosition) {
            left.push_back(req);
        } else {
            right.push_back(req);
        }
    }
    
    std::sort(left.begin(), left.end());
    std::sort(right.begin(), right.end());
    
    int current = currentPosition;
    sequence.push_back(current);
    
    if (movingUp) {
        // Primero hacia arriba
        for (int req : right) {
            totalMovement += abs(req - current);
            current = req;
            sequence.push_back(current);
        }
        
        // Llegar al final si es necesario
        if (current < maxCylinder && !right.empty()) {
            totalMovement += maxCylinder - current;
            current = maxCylinder;
        }
        
        // Luego hacia abajo
        std::reverse(left.begin(), left.end());
        for (int req : left) {
            totalMovement += abs(current - req);
            current = req;
            sequence.push_back(current);
        }
    } else {
        // Primero hacia abajo
        std::reverse(left.begin(), left.end());
        for (int req : left) {
            totalMovement += abs(current - req);
            current = req;
            sequence.push_back(current);
        }
        
        // Llegar al inicio si es necesario
        if (current > 0 && !left.empty()) {
            totalMovement += current;
            current = 0;
        }
        
        // Luego hacia arriba
        for (int req : right) {
            totalMovement += abs(req - current);
            current = req;
            sequence.push_back(current);
        }
    }
}
