#include "../include/Process.h"
#include <iostream>
#include <algorithm>

Process::Process(int id, std::string n, int burst, int arrival, int prio)
    : pid(id), name(n), burstTime(burst), arrivalTime(arrival), priority(prio),
      state(ProcessState::NEW), remainingTime(burst), waitingTime(0),
      turnaroundTime(0), responseTime(-1), completionTime(0) {}

ProcessManager::ProcessManager() : nextPid(1), currentTime(0) {}

ProcessManager::~ProcessManager() {
    for (auto p : processes) {
        delete p;
    }
}

Process* ProcessManager::createProcess(std::string name, int burst, int arrival, int priority) {
    Process* p = new Process(nextPid++, name, burst, arrival, priority);
    p->state = ProcessState::READY;
    processes.push_back(p);
    return p;
}

bool ProcessManager::suspendProcess(int pid) {
    Process* p = getProcess(pid);
    if (p && p->state != ProcessState::TERMINATED) {
        p->state = ProcessState::SUSPENDED;
        return true;
    }
    return false;
}

bool ProcessManager::resumeProcess(int pid) {
    Process* p = getProcess(pid);
    if (p && p->state == ProcessState::SUSPENDED) {
        p->state = ProcessState::READY;
        return true;
    }
    return false;
}

bool ProcessManager::terminateProcess(int pid) {
    Process* p = getProcess(pid);
    if (p && p->state != ProcessState::TERMINATED) {
        p->state = ProcessState::TERMINATED;
        p->completionTime = currentTime;
        return true;
    }
    return false;
}

Process* ProcessManager::getProcess(int pid) {
    for (auto p : processes) {
        if (p->pid == pid) return p;
    }
    return nullptr;
}

std::vector<Process*> ProcessManager::getReadyProcesses() {
    std::vector<Process*> ready;
    for (auto p : processes) {
        if (p->state == ProcessState::READY) {
            ready.push_back(p);
        }
    }
    return ready;
}

void ProcessManager::printStatistics() {
    std::vector<Process*> terminated;
    for (auto p : processes) {
        if (p->state == ProcessState::TERMINATED) {
            terminated.push_back(p);
        }
    }
    
    if (terminated.empty()) {
        std::cout << "No hay procesos terminados" << std::endl;
        return;
    }
    
    double totalWaiting = 0, totalTurnaround = 0, totalResponse = 0;
    
    std::cout << "\n=== Estadisticas de Procesos ===" << std::endl;
    std::cout << "PID\tNombre\t\tEspera\tRetorno\tRespuesta" << std::endl;
    
    for (auto p : terminated) {
        std::cout << p->pid << "\t" << p->name << "\t\t" 
                  << p->waitingTime << "\t" << p->turnaroundTime << "\t" 
                  << p->responseTime << std::endl;
        totalWaiting += p->waitingTime;
        totalTurnaround += p->turnaroundTime;
        totalResponse += p->responseTime;
    }
    
    std::cout << "\nPromedios:" << std::endl;
    std::cout << "Tiempo de espera: " << totalWaiting / terminated.size() << std::endl;
    std::cout << "Tiempo de retorno: " << totalTurnaround / terminated.size() << std::endl;
    std::cout << "Tiempo de respuesta: " << totalResponse / terminated.size() << std::endl;
}

// Round Robin Scheduler
RoundRobinScheduler::RoundRobinScheduler(ProcessManager* manager, int q)
    : pm(manager), quantum(q) {}

void RoundRobinScheduler::run() {
    std::cout << "\n=== Ejecutando Round Robin (Quantum=" << quantum << ") ===" << std::endl;
    
    while (true) {
        // Agregar procesos que llegaron
        for (auto p : pm->getReadyProcesses()) {
            bool inQueue = false;
            std::queue<Process*> temp = readyQueue;
            while (!temp.empty()) {
                if (temp.front()->pid == p->pid) {
                    inQueue = true;
                    break;
                }
                temp.pop();
            }
            if (!inQueue && p->arrivalTime <= pm->currentTime) {
                readyQueue.push(p);
            }
        }
        
        if (readyQueue.empty()) {
            bool pending = false;
            for (auto p : pm->getProcesses()) {
                if (p->arrivalTime > pm->currentTime && p->state != ProcessState::TERMINATED) {
                    pending = true;
                    break;
                }
            }
            if (!pending) break;
            pm->currentTime++;
            continue;
        }
        
        Process* p = readyQueue.front();
        readyQueue.pop();
        p->state = ProcessState::RUNNING;
        
        if (p->responseTime == -1) {
            p->responseTime = pm->currentTime - p->arrivalTime;
        }
        
        int timeToExecute = std::min(quantum, p->remainingTime);
        
        std::cout << "T=" << pm->currentTime << ": Ejecutando " << p->name 
                  << " (PID=" << p->pid << ", Restante=" << p->remainingTime << ")" << std::endl;
        
        for (int i = 0; i < timeToExecute; i++) {
            pm->currentTime++;
            p->remainingTime--;
            
            // Actualizar tiempo de espera
            for (auto proc : pm->getProcesses()) {
                if (proc->state == ProcessState::READY && proc->pid != p->pid) {
                    if (proc->arrivalTime <= pm->currentTime) {
                        proc->waitingTime++;
                    }
                }
            }
        }
        
        if (p->remainingTime == 0) {
            p->state = ProcessState::TERMINATED;
            p->completionTime = pm->currentTime;
            p->turnaroundTime = p->completionTime - p->arrivalTime;
            std::cout << "T=" << pm->currentTime << ": " << p->name << " TERMINADO" << std::endl;
        } else {
            p->state = ProcessState::READY;
            readyQueue.push(p);
        }
    }
}

// SJF Scheduler
SJFScheduler::SJFScheduler(ProcessManager* manager) : pm(manager) {}

void SJFScheduler::run() {
    std::cout << "\n=== Ejecutando SJF (Shortest Job First) ===" << std::endl;
    
    while (true) {
        std::vector<Process*> ready;
        for (auto p : pm->getProcesses()) {
            if (p->arrivalTime <= pm->currentTime && p->state == ProcessState::READY) {
                ready.push_back(p);
            }
        }
        
        if (ready.empty()) {
            bool pending = false;
            for (auto p : pm->getProcesses()) {
                if (p->arrivalTime > pm->currentTime && p->state != ProcessState::TERMINATED) {
                    pending = true;
                    break;
                }
            }
            if (!pending) break;
            pm->currentTime++;
            continue;
        }
        
        // Seleccionar proceso con menor burst time
        std::sort(ready.begin(), ready.end(), 
                  [](Process* a, Process* b) { return a->remainingTime < b->remainingTime; });
        
        Process* p = ready[0];
        p->state = ProcessState::RUNNING;
        
        if (p->responseTime == -1) {
            p->responseTime = pm->currentTime - p->arrivalTime;
        }
        
        std::cout << "T=" << pm->currentTime << ": Ejecutando " << p->name 
                  << " (PID=" << p->pid << ", Burst=" << p->remainingTime << ")" << std::endl;
        
        while (p->remainingTime > 0) {
            pm->currentTime++;
            p->remainingTime--;
            
            // Actualizar tiempo de espera
            for (auto proc : pm->getProcesses()) {
                if (proc->state == ProcessState::READY && proc->pid != p->pid) {
                    if (proc->arrivalTime <= pm->currentTime) {
                        proc->waitingTime++;
                    }
                }
            }
        }
        
        p->state = ProcessState::TERMINATED;
        p->completionTime = pm->currentTime;
        p->turnaroundTime = p->completionTime - p->arrivalTime;
        std::cout << "T=" << pm->currentTime << ": " << p->name << " TERMINADO" << std::endl;
    }
}
