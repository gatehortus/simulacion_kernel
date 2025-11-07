#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <string>
#include "../include/Process.h"
#include "../include/Memory.h"
#include "../include/Synchronization.h"
#include "../include/Disk.h"
#include "../include/IO.h"

using namespace std;

void printMenu() {
    cout << "\n========================================" << endl;
    cout << "  SIMULADOR DE NUCLEO DE SO" << endl;
    cout << "========================================" << endl;
    cout << "1. Gestion de Procesos" << endl;
    cout << "2. Memoria Virtual" << endl;
    cout << "3. Sincronizacion" << endl;
    cout << "4. Planificacion de Disco" << endl;
    cout << "5. Entrada/Salida" << endl;
    cout << "6. Ejecutar Scripts de Prueba" << endl;
    cout << "0. Salir" << endl;
    cout << "========================================" << endl;
    cout << "Opcion: ";
}

void processMenu() {
    cout << "\n--- Menu de Procesos ---" << endl;
    cout << "1. Planificador Round Robin" << endl;
    cout << "2. Planificador SJF" << endl;
    cout << "3. Cargar desde script" << endl;
    cout << "0. Volver" << endl;
    cout << "Opcion: ";
}

void memoryMenu() {
    cout << "\n--- Menu de Memoria ---" << endl;
    cout << "1. Algoritmo LRU" << endl;
    cout << "2. Algoritmo FIFO" << endl;
    cout << "3. Algoritmo Working Set" << endl;
    cout << "4. Comparar algoritmos" << endl;
    cout << "5. Cargar desde script" << endl;
    cout << "0. Volver" << endl;
    cout << "Opcion: ";
}

void diskMenu() {
    cout << "\n--- Menu de Disco ---" << endl;
    cout << "1. Algoritmo FCFS" << endl;
    cout << "2. Algoritmo SSTF" << endl;
    cout << "3. Algoritmo SCAN" << endl;
    cout << "4. Comparar algoritmos" << endl;
    cout << "5. Cargar desde script" << endl;
    cout << "0. Volver" << endl;
    cout << "Opcion: ";
}

vector<int> loadPageReferences(const string& filename) {
    vector<int> pages;
    ifstream file(filename);
    if (file.is_open()) {
        int page;
        while (file >> page) {
            pages.push_back(page);
        }
        file.close();
    }
    return pages;
}

void runMemorySimulation(MemoryManager* mm, const vector<int>& pages) {
    cout << "\nAccediendo a paginas: ";
    for (int page : pages) {
        cout << page << " ";
    }
    cout << endl;
    
    for (int page : pages) {
        bool hit = mm->accessPage(page);
        cout << "Pagina " << page << ": " << (hit ? "HIT" : "FAULT") << endl;
    }
    
    mm->printStatistics();
}

void testProcessScheduling() {
    ProcessManager pm;
    
    // Crear procesos de prueba
    pm.createProcess("P1", 5, 0, 1);
    pm.createProcess("P2", 3, 1, 2);
    pm.createProcess("P3", 8, 2, 1);
    pm.createProcess("P4", 6, 3, 3);
    
    int option;
    processMenu();
    cin >> option;
    
    switch(option) {
        case 1: {
            RoundRobinScheduler rr(&pm, 2);
            rr.run();
            pm.printStatistics();
            break;
        }
        case 2: {
            SJFScheduler sjf(&pm);
            sjf.run();
            pm.printStatistics();
            break;
        }
        case 3: {
            cout << "Ingrese nombre del archivo: ";
            string filename;
            cin >> filename;
            
            ifstream file(filename);
            if (file.is_open()) {
                ProcessManager pm2;
                string name;
                int burst, arrival, priority;
                
                while (file >> name >> burst >> arrival >> priority) {
                    pm2.createProcess(name, burst, arrival, priority);
                }
                file.close();
                
                RoundRobinScheduler rr(&pm2, 2);
                rr.run();
                pm2.printStatistics();
            } else {
                cout << "Error al abrir archivo" << endl;
            }
            break;
        }
    }
}

void testMemory() {
    int option;
    memoryMenu();
    cin >> option;
    
    if (option == 0) return;
    
    int frames = 3;
    cout << "Numero de marcos (default 3): ";
    cin >> frames;
    
    vector<int> pages = {1, 2, 3, 4, 1, 2, 5, 1, 2, 3, 4, 5};
    
    if (option == 5) {
        cout << "Ingrese nombre del archivo: ";
        string filename;
        cin >> filename;
        pages = loadPageReferences(filename);
        if (pages.empty()) {
            cout << "Error al cargar archivo" << endl;
            return;
        }
    }
    
    switch(option) {
        case 1: {
            LRUMemory lru(frames);
            runMemorySimulation(&lru, pages);
            break;
        }
        case 2: {
            FIFOMemory fifo(frames);
            runMemorySimulation(&fifo, pages);
            break;
        }
        case 3: {
            WorkingSetMemory ws(frames, 4);
            runMemorySimulation(&ws, pages);
            break;
        }
        case 4: {
            cout << "\n=== COMPARACION DE ALGORITMOS ===" << endl;
            
            LRUMemory lru(frames);
            FIFOMemory fifo(frames);
            WorkingSetMemory ws(frames, 4);
            
            runMemorySimulation(&lru, pages);
            runMemorySimulation(&fifo, pages);
            runMemorySimulation(&ws, pages);
            break;
        }
    }
}

void testSynchronization() {
    cout << "\n--- Menu de Sincronizacion ---" << endl;
    cout << "1. Productor-Consumidor" << endl;
    cout << "2. Cena de los Filosofos" << endl;
    cout << "Opcion: ";
    
    int option;
    cin >> option;
    
    switch(option) {
        case 1: {
            ProducerConsumer pc;
            pc.runSimulation(2, 2, 5);
            break;
        }
        case 2: {
            DiningPhilosophers dp;
            dp.runSimulation();
            break;
        }
    }
}

void testDisk() {
    int option;
    diskMenu();
    cin >> option;
    
    if (option == 0) return;
    
    vector<int> requests = {98, 183, 37, 122, 14, 124, 65, 67};
    int initial = 53;
    
    if (option == 5) {
        cout << "Ingrese nombre del archivo: ";
        string filename;
        cin >> filename;
        
        ifstream file(filename);
        if (file.is_open()) {
            file >> initial;
            requests.clear();
            int cyl;
            while (file >> cyl) {
                requests.push_back(cyl);
            }
            file.close();
        }
    }
    
    switch(option) {
        case 1: {
            FCFSDisk fcfs(initial);
            fcfs.setRequests(requests);
            fcfs.schedule();
            fcfs.printResults();
            break;
        }
        case 2: {
            SSTFDisk sstf(initial);
            sstf.setRequests(requests);
            sstf.schedule();
            sstf.printResults();
            break;
        }
        case 3: {
            SCANDisk scan(initial, 200);
            scan.setRequests(requests);
            scan.schedule();
            scan.printResults();
            break;
        }
        case 4: {
            cout << "\n=== COMPARACION DE ALGORITMOS ===" << endl;
            
            FCFSDisk fcfs(initial);
            fcfs.setRequests(requests);
            fcfs.schedule();
            fcfs.printResults();
            
            SSTFDisk sstf(initial);
            sstf.setRequests(requests);
            sstf.schedule();
            sstf.printResults();
            
            SCANDisk scan(initial, 200);
            scan.setRequests(requests);
            scan.schedule();
            scan.printResults();
            
            cout << "\n=== RESUMEN ===" << endl;
            cout << "FCFS: " << fcfs.getTotalMovement() << " cilindros" << endl;
            cout << "SSTF: " << sstf.getTotalMovement() << " cilindros" << endl;
            cout << "SCAN: " << scan.getTotalMovement() << " cilindros" << endl;
            break;
        }
    }
}

void testIO() {
    cout << "\n--- Menu de E/S ---" << endl;
    cout << "1. Simulacion de Impresora" << endl;
    cout << "2. Buffer Compartido" << endl;
    cout << "Opcion: ";
    
    int option;
    cin >> option;
    
    switch(option) {
        case 1: {
            Printer printer;
            
            printer.addRequest(1, "Documento 1", 3, 0);
            printer.addRequest(2, "Documento 2", 1, 1);
            printer.addRequest(3, "Documento 3", 2, 2);
            printer.addRequest(4, "Documento 4", 1, 3);
            
            printer.processRequest();
            break;
        }
        case 2: {
            SharedBuffer buffer;
            
            buffer.write("Dato A");
            buffer.write("Dato B");
            buffer.write("Dato C");
            
            buffer.read();
            buffer.read();
            
            buffer.write("Dato D");
            buffer.read();
            buffer.read();
            break;
        }
    }
}

void runAllScripts() {
    cout << "\n=== EJECUTANDO TODOS LOS SCRIPTS ===" << endl;
    
    // Procesos
    cout << "\n### PROCESOS ###" << endl;
    ifstream procFile("scripts/proc_test.txt");
    if (procFile.is_open()) {
        ProcessManager pm;
        string name;
        int burst, arrival, priority;
        
        while (procFile >> name >> burst >> arrival >> priority) {
            pm.createProcess(name, burst, arrival, priority);
        }
        procFile.close();
        
        RoundRobinScheduler rr(&pm, 2);
        rr.run();
        pm.printStatistics();
    }
    
    // Memoria
    cout << "\n### MEMORIA ###" << endl;
    vector<int> pages = loadPageReferences("scripts/mem_test.txt");
    if (!pages.empty()) {
        LRUMemory lru(3);
        FIFOMemory fifo(3);
        WorkingSetMemory ws(3, 4);
        
        runMemorySimulation(&lru, pages);
        runMemorySimulation(&fifo, pages);
        runMemorySimulation(&ws, pages);
    }
    
    // Disco
    cout << "\n### DISCO ###" << endl;
    ifstream diskFile("scripts/disk_test.txt");
    if (diskFile.is_open()) {
        int initial;
        vector<int> requests;
        
        diskFile >> initial;
        int cyl;
        while (diskFile >> cyl) {
            requests.push_back(cyl);
        }
        diskFile.close();
        
        FCFSDisk fcfs(initial);
        fcfs.setRequests(requests);
        fcfs.schedule();
        fcfs.printResults();
        
        SSTFDisk sstf(initial);
        sstf.setRequests(requests);
        sstf.schedule();
        sstf.printResults();
        
        SCANDisk scan(initial, 200);
        scan.setRequests(requests);
        scan.schedule();
        scan.printResults();
    }
}

int main() {
    int option;
    
    do {
        printMenu();
        cin >> option;
        
        switch(option) {
            case 1:
                testProcessScheduling();
                break;
            case 2:
                testMemory();
                break;
            case 3:
                testSynchronization();
                break;
            case 4:
                testDisk();
                break;
            case 5:
                testIO();
                break;
            case 6:
                runAllScripts();
                break;
            case 0:
                cout << "Saliendo..." << endl;
                break;
            default:
                cout << "Opcion invalida" << endl;
        }
    } while (option != 0);
    
    return 0;
}
