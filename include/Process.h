#ifndef PROCESS_H
#define PROCESS_H

#include <string>
#include <queue>
#include <vector>

enum class ProcessState {
    NEW,
    READY,
    RUNNING,
    BLOCKED,
    SUSPENDED,
    TERMINATED
};

class Process {
public:
    int pid;
    std::string name;
    int burstTime;
    int arrivalTime;
    int priority;
    ProcessState state;
    int remainingTime;
    int waitingTime;
    int turnaroundTime;
    int responseTime;
    int completionTime;

    Process(int id, std::string n, int burst, int arrival, int prio = 0);
};

class ProcessManager {
private:
    std::vector<Process*> processes;
    int nextPid;
    
public:
    int currentTime;
    
    ProcessManager();
    ~ProcessManager();
    
    Process* createProcess(std::string name, int burst, int arrival, int priority = 0);
    bool suspendProcess(int pid);
    bool resumeProcess(int pid);
    bool terminateProcess(int pid);
    Process* getProcess(int pid);
    std::vector<Process*> getReadyProcesses();
    void printStatistics();
};

class RoundRobinScheduler {
private:
    ProcessManager* pm;
    int quantum;
    std::queue<Process*> readyQueue;
    
public:
    RoundRobinScheduler(ProcessManager* manager, int q = 4);
    void run();
};

class SJFScheduler {
private:
    ProcessManager* pm;
    
public:
    SJFScheduler(ProcessManager* manager);
    void run();
};

#endif
