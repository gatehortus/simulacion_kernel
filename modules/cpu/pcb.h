@'
#pragma once
#include <string>
#include <vector>

struct PCB {
    int id = 0;
    int burst = 0;
    int remaining = 0;
    std::string state = "NEW"; // NEW, READY, RUNNING, WAITING, FINISHED
    std::vector<int> pages; // lista de páginas que usa el proceso (simulación)
    // metricas simples
    int arrival_time = 0;
    int finish_time = 0;
    int waiting_time = 0;
};
'@ | Out-File -Encoding utf8 modules\cpu\pcb.h
