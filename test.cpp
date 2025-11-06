#include <iostream>
#include <vector>
#include <random>
#include <fstream>
#include <sstream>
#include "disk_scheduler.h"
#include "philosophers.h"
#include "pff_memory.h"
#include "heap_allocator.h"

// Funciones auxiliares para generar datos de prueba
std::vector<int> generateRandomPageSequence(int length, int max_page) {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(0, max_page - 1);
    
    std::vector<int> sequence;
    for (int i = 0; i < length; i++) {
        sequence.push_back(dis(gen));
    }
    return sequence;
}

std::vector<DiskRequest> generateDiskRequests(int count, int max_cylinder) {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> cyl_dis(0, max_cylinder - 1);
    
    std::vector<DiskRequest> requests;
    for (int i = 0; i < count; i++) {
        requests.push_back({cyl_dis(gen), i, i});
    }
    return requests;
}

void testDiskScheduling() {
    std::cout << "\n╔═══════════════════════════════════════════════════╗" << std::endl;
    std::cout << "║     PRUEBA: ALGORITMOS DE PLANIFICACIÓN DE DISCO ║" << std::endl;
    std::cout << "╚═══════════════════════════════════════════════════╝\n" << std::endl;
    
    DiskScheduler disk(200, 50);
    
    // Generar solicitudes de prueba
    auto requests = generateDiskRequests(15, 200);
    
    std::cout << "Solicitudes generadas: ";
    for (const auto& req : requests) {
        disk.addRequest(req.cylinder, req.arrival_time, req.process_id);
        std::cout << req.cylinder << " ";
    }
    std::cout << "\nPosición inicial del cabezal: " << disk.getHead() << "\n" << std::endl;
    
    // Probar FCFS
    auto fcfs_stats = disk.scheduleFCFS();
    disk.printStats(fcfs_stats);
    std::cout << disk.visualizeDisk(fcfs_stats);
    
    // Probar SSTF
    disk.setHead(50);  // Reiniciar posición
    auto sstf_stats = disk.scheduleSSTF();
    disk.printStats(sstf_stats);
    std::cout << disk.visualizeDisk(sstf_stats);
    
    // Probar SCAN
    disk.setHead(50);
    auto scan_stats = disk.scheduleSCAN(true);
    disk.printStats(scan_stats);
    std::cout << disk.visualizeDisk(scan_stats);
    
    // Comparativa
    std::cout << "\n╔════════════════════════════════════════╗" << std::endl;
    std::cout << "║       COMPARATIVA DE ALGORITMOS       ║" << std::endl;
    std::cout << "╠════════════╦═══════════╦═══════════════╣" << std::endl;
    std::cout << "║ Algoritmo  ║ Movimiento║ Tiempo Prom.  ║" << std::endl;
    std::cout << "╠════════════╬═══════════╬═══════════════╣" << std::endl;
    std::cout << "║ FCFS       ║ " << std::setw(9) << fcfs_stats.total_head_movement 
              << " ║ " << std::setw(13) << std::fixed << std::setprecision(2) 
              << fcfs_stats.avg_wait_time << " ║" << std::endl;
    std::cout << "║ SSTF       ║ " << std::setw(9) << sstf_stats.total_head_movement 
              << " ║ " << std::setw(13) << sstf_stats.avg_wait_time << " ║" << std::endl;
    std::cout << "║ SCAN       ║ " << std::setw(9) << scan_stats.total_head_movement 
              << " ║ " << std::setw(13) << scan_stats.avg_wait_time << " ║" << std::endl;
    std::cout << "╚════════════╩═══════════╩═══════════════╝" << std::endl;
}

void testPhilosophers() {
    std::cout << "\n╔═══════════════════════════════════════════════════╗" << std::endl;
    std::cout << "║        PRUEBA: CENA DE LOS FILÓSOFOS              ║" << std::endl;
    std::cout << "╚═══════════════════════════════════════════════════╝\n" << std::endl;
    
    DiningPhilosophers dining(5);
    
    std::cout << "Iniciando simulación con 5 filósofos...\n" << std::endl;
    dining.start(3, true);  // 3 comidas por filósofo, versión segura
    
    // Esperar a que terminen
    std::this_thread::sleep_for(std::chrono::seconds(5));
    
    dining.stop();
    dining.printStats();
    std::cout << dining.visualizeTable();
}

void testAdvancedMemory() {
    std::cout << "\n╔═══════════════════════════════════════════════════╗" << std::endl;
    std::cout << "║    PRUEBA: ALGORITMOS AVANZADOS DE MEMORIA        ║" << std::endl;
    std::cout << "╚═══════════════════════════════════════════════════╝\n" << std::endl;
    
    // Generar secuencia de páginas
    auto page_sequence = generateRandomPageSequence(100, 20);
    
    // Probar PFF
    std::cout << "\n--- Probando PFF (Page Fault Frequency) ---\n" << std::endl;
    PFFMemoryManager pff(10, 5, 0.4, 0.1);
    
    for (int page : page_sequence) {
        pff.accessPage(page);
    }
    
    pff.printStats();
    std::cout << pff.visualizeMemory();
    
    // Probar Working Set
    std::cout << "\n--- Probando Working Set ---\n" << std::endl;
    WorkingSetMemoryManager ws(10, 5);
    
    for (int page : page_sequence) {
        ws.accessPage(page);
    }
    
    ws.printStats();
    std::cout << ws.visualizeMemory();
    
    // Comparativa
    std::cout << "\n╔════════════════════════════════════════════╗" << std::endl;
    std::cout << "║       COMPARATIVA PFF vs Working Set       ║" << std::endl;
    std::cout << "╠═══════════════╦═══════════╦════════════════╣" << std::endl;
    std::cout << "║ Algoritmo     ║ Fallos    ║ Tasa de Fallos ║" << std::endl;
    std::cout << "╠═══════════════╬═══════════╬════════════════╣" << std::endl;
    std::cout << "║ PFF           ║ " << std::setw(9) << pff.getPageFaults() 
              << " ║ " << std::setw(13) << std::fixed << std::setprecision(2)
              << (pff.getStats().fault_rate * 100) << "% ║" << std::endl;
    std::cout << "║ Working Set   ║ " << std::setw(9) << ws.getPageFaults() 
              << " ║ " << std::setw(13) << (ws.getStats().fault_rate * 100) << "% ║" << std::endl;
    std::cout << "╚═══════════════╩═══════════╩════════════════╝" << std::endl;
}

void testHeapAllocators() {
    std::cout << "\n╔═══════════════════════════════════════════════════╗" << std::endl;
    std::cout << "║      PRUEBA: ASIGNADORES DE MEMORIA HEAP          ║" << std::endl;
    std::cout << "╚═══════════════════════════════════════════════════╝\n" << std::endl;
    
    // Probar Buddy System
    std::cout << "\n--- Probando Buddy System ---\n" << std::endl;
    BuddyAllocator buddy(1024);
    
    std::vector<void*> buddy_ptrs;
    buddy_ptrs.push_back(buddy.allocate(64));
    buddy_ptrs.push_back(buddy.allocate(128));
    buddy_ptrs.push_back(buddy.allocate(32));
    buddy_ptrs.push_back(buddy.allocate(256));
    buddy_ptrs.push_back(buddy.allocate(16));
    
    buddy.printStats();
    std::cout << buddy.visualizeHeap();
    
    // Liberar algunos bloques
    buddy.deallocate(buddy_ptrs[1]);
    buddy.deallocate(buddy_ptrs[3]);
    
    std::cout << "\nDespués de liberar 2 bloques:\n";
    buddy.printStats();
    std::cout << buddy.visualizeHeap();
    
    // Probar Segregated Free List
    std::cout << "\n--- Probando Segregated Free List ---\n" << std::endl;
    SegregatedAllocator segregated(1024);
    
    std::vector<void*> seg_ptrs;
    seg_ptrs.push_back(segregated.allocate(8));
    seg_ptrs.push_back(segregated.allocate(64));
    seg_ptrs.push_back(segregated.allocate(16));
    seg_ptrs.push_back(segregated.allocate(128));
    seg_ptrs.push_back(segregated.allocate(32));
    
    segregated.printStats();
    std::cout << segregated.visualizeHeap();
    
    // Comparativa
    std::cout << "\n╔═══════════════════════════════════════════════════════╗" << std::endl;
    std::cout << "║    COMPARATIVA: Buddy System vs Segregated            ║" << std::endl;
    std::cout << "╠══════════════════╦═══════════════╦════════════════════╣" << std::endl;
    std::cout << "║ Métrica          ║ Buddy System  ║ Segregated         ║" << std::endl;
    std::cout << "╠══════════════════╬═══════════════╬════════════════════╣" << std::endl;
    std::cout << "║ Frag. Externa    ║ " << std::setw(12) << std::fixed 
              << std::setprecision(2) << (buddy.getStats().fragmentation_external * 100) 
              << "% ║ " << std::setw(17) << (segregated.getStats().fragmentation_external * 100) 
              << "% ║" << std::endl;
    std::cout << "║ Frag. Interna    ║ " << std::setw(10) 
              << buddy.getStats().fragmentation_internal << " B ║ " << std::setw(15) 
              << segregated.getStats().fragmentation_internal << " B ║" << std::endl;
    std::cout << "║ Asignaciones     ║ " << std::setw(13) << buddy.getStats().allocations 
              << " ║ " << std::setw(18) << segregated.getStats().allocations << " ║" << std::endl;
    std::cout << "╚══════════════════╩═══════════════╩════════════════════╝" << std::endl;
}

void exportResultsToCSV() {
    std::ofstream csv("resultados_simulacion.csv");
    
    csv << "Módulo,Algoritmo,Métrica,Valor\n";
    
    // Datos de disco
    DiskScheduler disk(200, 50);
    auto requests = generateDiskRequests(15, 200);
    for (const auto& req : requests) {
        disk.addRequest(req.cylinder, req.arrival_time, req.process_id);
    }
    
    auto fcfs = disk.scheduleFCFS();
    disk.setHead(50);
    auto sstf = disk.scheduleSSTF();
    disk.setHead(50);
    auto scan = disk.scheduleSCAN(true);
    
    csv << "Disco,FCFS,Movimiento Total," << fcfs.total_head_movement << "\n";
    csv << "Disco,SSTF,Movimiento Total," << sstf.total_head_movement << "\n";
    csv << "Disco,SCAN,Movimiento Total," << scan.total_head_movement << "\n";
    
    // Datos de memoria
    auto pages = generateRandomPageSequence(100, 20);
    PFFMemoryManager pff(10, 5);
    WorkingSetMemoryManager ws(10, 5);
    
    for (int page : pages) {
        pff.accessPage(page);
        ws.accessPage(page);
    }
    
    csv << "Memoria,PFF,Fallos de Página," << pff.getPageFaults() << "\n";
    csv << "Memoria,Working Set,Fallos de Página," << ws.getPageFaults() << "\n";
    csv << "Memoria,PFF,Tasa de Fallos," << (pff.getStats().fault_rate * 100) << "\n";
    csv << "Memoria,Working Set,Tasa de Fallos," << (ws.getStats().fault_rate * 100) << "\n";
    
    // Datos de heap
    BuddyAllocator buddy(1024);
    SegregatedAllocator seg(1024);
    
    for (int i = 0; i < 5; i++) {
        buddy.allocate(32 * (i + 1));
        seg.allocate(32 * (i + 1));
    }
    
    csv << "Heap,Buddy System,Fragmentación Externa," 
        << (buddy.getStats().fragmentation_external * 100) << "\n";
    csv << "Heap,Segregated,Fragmentación Externa," 
        << (seg.getStats().fragmentation_external * 100) << "\n";
    
    csv.close();
    std::cout << "\n✅ Resultados exportados a 'resultados_simulacion.csv'\n" << std::endl;
}

int main() {
    std::cout << "╔════════════════════════════════════════════════════════╗" << std::endl;
    std::cout << "║                                                        ║" << std::endl;
    std::cout << "║   SIMULACIÓN AVANZADA DE NÚCLEO DE SISTEMA OPERATIVO  ║" << std::endl;
    std::cout << "║                                                        ║" << std::endl;
    std::cout << "║   Proyecto Final - Sistemas Operativos                ║" << std::endl;
    std::cout << "║   Gabriel Atehortua & Mateo Sanz                      ║" << std::endl;
    std::cout << "║                                                        ║" << std::endl;
    std::cout << "╚════════════════════════════════════════════════════════╝\n" << std::endl;
    
    int option;
    do {
        std::cout << "\n╔════════════════ MENÚ PRINCIPAL ═══════════════════╗" << std::endl;
        std::cout << "║                                                   ║" << std::endl;
        std::cout << "║  1. Probar Planificación de Disco                ║" << std::endl;
        std::cout << "║  2. Probar Cena de los Filósofos                 ║" << std::endl;
        std::cout << "║  3. Probar Algoritmos Avanzados de Memoria       ║" << std::endl;
        std::cout << "║  4. Probar Asignadores de Heap                   ║" << std::endl;
        std::cout << "║  5. Ejecutar TODAS las pruebas                   ║" << std::endl;
        std::cout << "║  6. Exportar resultados a CSV                    ║" << std::endl;
        std::cout << "║  0. Salir                                        ║" << std::endl;
        std::cout << "║                                                   ║" << std::endl;
        std::cout << "╚═══════════════════════════════════════════════════╝" << std::endl;
        std::cout << "\nSeleccione una opción: ";
        std::cin >> option;
        
        switch (option) {
            case 1:
                testDiskScheduling();
                break;
            case 2:
                testPhilosophers();
                break;
            case 3:
                testAdvancedMemory();
                break;
            case 4:
                testHeapAllocators();
                break;
            case 5:
                testDiskScheduling();
                testPhilosophers();
                testAdvancedMemory();
                testHeapAllocators();
                break;
            case 6:
                exportResultsToCSV();
                break;
            case 0:
                std::cout << "\n¡Hasta luego! 👋\n" << std::endl;
                break;
            default:
                std::cout << "\n❌ Opción inválida. Intente de nuevo.\n" << std::endl;
        }
    } while (option != 0);
    
    return 0;
}
