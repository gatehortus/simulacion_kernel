@'
#include <iostream>
#include <string>
#include <sstream>
#include <vector>
#include <algorithm>
#include <fstream>
#include "modules/cpu/pcb.h"
#include "modules/cpu/scheduler.h"
#include "modules/mem/memory.h"
#include "modules/sync/sync.h"
#include "modules/io/io.h"


Scheduler scheduler;
MemoryManager memmgr(4); 
SyncManager syncmgr;
IOManager iomgr;

void print_help() {
    std::cout << "Comandos disponibles:\n"
              << "  new <burst> [pages...]  - crear proceso con ráfaga y lista de páginas (ej: new 5 1 2 3)\n"
              << "  ps                      - listar procesos\n"
              << "  tick                    - avanzar 1 unidad (scheduler)\n"
              << "  run <n>                 - ejecutar n ticks\n"
              << "  kill <id>               - terminar proceso id\n"
              << "  memstat                 - mostrar estado de memoria\n"
              << "  memaccess <pid> <page>  - simular acceso a página por PID\n"
              << "  produce                 - producir (productor-consumidor)\n"
              << "  consume                 - consumir (productor-consumidor)\n"
              << "  stat                    - estado del buffer (productor-consumidor)\n"
              << "  export                  - exportar métricas a data/resultados.csv\n"
              << "  help                    - mostrar ayuda\n"
              << "  exit                    - salir\n";
}

int main() {
    std::cout << "Kernel-Sim CLI (Entrega 2) - C++ (Windows)\n";
    print_help();
    std::string line;
    while (true) {
        std::cout << "> ";
        if (!std::getline(std::cin, line)) break;
        if (line.empty()) continue;

        std::istringstream iss(line);
        std::string cmd; iss >> cmd;

        if (cmd == "new") {
            int burst; if (!(iss >> burst)) { std::cout << "Uso: new <burst> [pages...]\n"; continue; }
            PCB p;
            p.id = scheduler.next_id();
            p.burst = burst;
            p.remaining = burst;
            p.state = "READY";
            int page;
            while (iss >> page) p.pages.push_back(page);
            scheduler.addProcess(p);
            std::cout << "Proceso " << p.id << " creado (burst=" << burst << ")\n";
        } else if (cmd == "ps") {
            scheduler.printProcesses();
        } else if (cmd == "tick") {
            scheduler.tick(memmgr, iomgr);
        } else if (cmd == "run") {
            int n; if (!(iss >> n)) { std::cout << "Uso: run <n>\n"; continue; }
            for (int i=0;i<n;i++) scheduler.tick(memmgr, iomgr);
        } else if (cmd == "kill") {
            int id; if (!(iss >> id)) { std::cout << "Uso: kill <id>\n"; continue; }
            scheduler.killProcess(id);
        } else if (cmd == "memstat") {
            memmgr.showStatus();
        } else if (cmd == "memaccess") {
            int pid, page; if (!(iss >> pid >> page)) { std::cout << "Uso: memaccess <pid> <page>\n"; continue; }
            memmgr.accessPage(pid, page);
        } else if (cmd == "produce") {
            syncmgr.produce();
        } else if (cmd == "consume") {
            syncmgr.consume();
        } else if (cmd == "stat") {
            syncmgr.printStatus();
        } else if (cmd == "export") {
            std::ofstream f("data/resultados.csv");
            f << "frame_count,failures\n";
            auto rows = memmgr.getStatsRows();
            for (auto &r: rows) f << r.first << "," << r.second << "\n";
            f.close();
            std::cout << "Exportado a data/resultados.csv\n";
        } else if (cmd == "help") {
            print_help();
        } else if (cmd == "exit") {
            break;
        } else {
            std::cout << "Comando no reconocido. Escribe 'help' para ver comandos.\n";
        }
    }
    return 0;
}
'@ | Out-File -Encoding utf8 main.cpp
