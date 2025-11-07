#ifndef DISK_H
#define DISK_H

#include <vector>
#include <string>

class DiskScheduler {
protected:
    int currentPosition;
    std::vector<int> requests;
    std::vector<int> sequence;
    int totalMovement;
    
public:
    DiskScheduler(int initial);
    virtual ~DiskScheduler() {}
    virtual void schedule() = 0;
    virtual std::string getName() = 0;
    
    void addRequest(int cylinder);
    void setRequests(std::vector<int> req);
    int getTotalMovement() { return totalMovement; }
    std::vector<int> getSequence() { return sequence; }
    void printResults();
};

class FCFSDisk : public DiskScheduler {
public:
    FCFSDisk(int initial);
    void schedule() override;
    std::string getName() override { return "FCFS"; }
};

class SSTFDisk : public DiskScheduler {
public:
    SSTFDisk(int initial);
    void schedule() override;
    std::string getName() override { return "SSTF"; }
};

class SCANDisk : public DiskScheduler {
private:
    bool movingUp;
    int maxCylinder;
    
public:
    SCANDisk(int initial, int max = 200);
    void schedule() override;
    std::string getName() override { return "SCAN"; }
};

#endif
