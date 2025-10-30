
#pragma once
#include "pcb.h"
#include <queue>
#include <vector>
#include <iostream>
#include "../mem/memory.h"
#include "../io/io.h"

class Scheduler {
private:
    std::queue<PCB> ready;
    std::vector<PCB> finished;
    int quantum = 2;
    int current_time = 0;
    int id_counter = 1;
public:
    int next_id(){ return id_counter++; }
    void addProcess(const PCB &p) {
        PCB copy = p;
        copy.state = "READY";
        copy.arrival_time = current_time;
        ready.push(copy);
    }
    void tick(MemoryManager &memmgr, IOManager &iomgr) {
        current_time++;
        if (ready.empty()) { std::cout << "[Time " << current_time << "] CPU idle\n"; return; }
        PCB p = ready.front(); ready.pop();
        p.state = "RUNNING";
        std::cout << "[Time " << current_time << "] Ejecutando PID=" << p.id << " remaining=" << p.remaining << "\n";
        int exec = std::min(quantum, p.remaining);
        // Simular ejecucion y uso de paginas
        for (int i=0;i<exec;i++) {
            if (!p.pages.empty()) {
                int page = p.pages[i % p.pages.size()];
                memmgr.accessPage(p.id, page);
            }
        }
        p.remaining -= exec;
        if (p.remaining > 0) {
            p.state = "READY";
            ready.push(p);
        } else {
            p.state = "FINISHED";
            p.finish_time = current_time;
            finished.push_back(p);
            std::cout << "Proceso " << p.id << " terminado en t=" << current_time << "\n";
        }
    }
    void runAll(MemoryManager &memmgr, IOManager &iomgr) {
        while (!ready.empty()) tick(memmgr, iomgr);
    }
    void printProcesses() {
        std::cout << "Procesos en cola:\n";
        std::queue<PCB> tmp = ready;
        while (!tmp.empty()) {
            PCB p = tmp.front(); tmp.pop();
            std::cout << "  PID="<<p.id<<" state="<<p.state<<" remaining="<<p.remaining<<"\n";
        }
        if (ready.empty()) std::cout << "  (ninguno)\n";
        std::cout << "Procesos finalizados: " << finished.size() << "\n";
    }
    void killProcess(int id) {
        std::queue<PCB> newq;
        bool found=false;
        while(!ready.empty()){
            PCB p=ready.front(); ready.pop();
            if (p.id==id){ found=true; std::cout<<"Proceso "<<id<<" eliminado\n"; continue; }
            newq.push(p);
        }
        ready = std::move(newq);
        if(!found) std::cout<<"PID no encontrado en READY\n";
    }
};

