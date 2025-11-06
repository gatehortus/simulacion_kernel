#include "pff_memory.h"
#include <iostream>
#include <iomanip>
#include <sstream>
#include <algorithm>

// ==================== PFF Memory Manager ====================

PFFMemoryManager::PFFMemoryManager(int max, int initial, double upper, double lower)
    : max_frames(max), min_frames(2), current_frames(initial),
      upper_threshold(upper), lower_threshold(lower),
      page_faults(0), page_hits(0), frame_adjustments(0),
      last_fault_time(0), current_time(0) {}

bool PFFMemoryManager::accessPage(int page_number) {
    current_time++;
    
    // Verificar si la página está en memoria
    if (resident_pages.find(page_number) != resident_pages.end()) {
        page_hits++;
        return true;  // Page hit
    }
    
    // Page fault
    page_faults++;
    
    // Calcular PFF y ajustar frames si es necesario
    double pff = calculatePFF();
    
    if (pff > upper_threshold && current_frames < max_frames) {
        current_frames++;
        frame_adjustments++;
    } else if (pff < lower_threshold && current_frames > min_frames) {
        current_frames--;
        frame_adjustments++;
    }
    
    // Si hay espacio, agregar la página
    if (resident_pages.size() < static_cast<size_t>(current_frames)) {
        resident_pages.insert(page_number);
        page_queue.push(page_number);
    } else {
        // Reemplazar página usando FIFO
        int victim = page_queue.front();
        page_queue.pop();
        resident_pages.erase(victim);
        
        resident_pages.insert(page_number);
        page_queue.push(page_number);
    }
    
    working_set_history.push_back(resident_pages.size());
    last_fault_time = current_time;
    
    return false;  // Page fault
}

double PFFMemoryManager::calculatePFF() {
    if (current_time == 0) return 0.0;
    
    int time_since_last_fault = current_time - last_fault_time;
    if (time_since_last_fault == 0) return 1.0;
    
    // PFF = 1 / tiempo entre fallos de página
    return 1.0 / time_since_last_fault;
}

void PFFMemoryManager::reset() {
    resident_pages.clear();
    while (!page_queue.empty()) page_queue.pop();
    page_faults = 0;
    page_hits = 0;
    frame_adjustments = 0;
    current_time = 0;
    last_fault_time = 0;
    working_set_history.clear();
}

PFFStats PFFMemoryManager::getStats() const {
    PFFStats stats;
    stats.algorithm = "PFF";
    stats.page_faults = page_faults;
    stats.page_hits = page_hits;
    stats.fault_rate = (page_faults + page_hits) > 0 ? 
        static_cast<double>(page_faults) / (page_faults + page_hits) : 0.0;
    stats.frame_adjustments = frame_adjustments;
    stats.working_set_sizes = working_set_history;
    return stats;
}

void PFFMemoryManager::printStats() const {
    std::cout << "\n╔═════════════════════════════════════════════╗" << std::endl;
    std::cout << "║   ESTADÍSTICAS - PFF (Page Fault Frequency) ║" << std::endl;
    std::cout << "╠═════════════════════════════════════════════╣" << std::endl;
    std::cout << "║ Fallos de página:      " << std::setw(18) << page_faults << " ║" << std::endl;
    std::cout << "║ Aciertos de página:    " << std::setw(18) << page_hits << " ║" << std::endl;
    std::cout << "║ Tasa de fallos:        " << std::setw(16) << std::fixed 
              << std::setprecision(2) << (getStats().fault_rate * 100) << "% ║" << std::endl;
    std::cout << "║ Ajustes de frames:     " << std::setw(18) << frame_adjustments << " ║" << std::endl;
    std::cout << "║ Frames actuales:       " << std::setw(18) << current_frames << " ║" << std::endl;
    std::cout << "║ Frames máximos:        " << std::setw(18) << max_frames << " ║" << std::endl;
    std::cout << "║ Umbral superior:       " << std::setw(16) << std::fixed 
              << std::setprecision(2) << (upper_threshold * 100) << "% ║" << std::endl;
    std::cout << "║ Umbral inferior:       " << std::setw(16) << std::fixed 
              << std::setprecision(2) << (lower_threshold * 100) << "% ║" << std::endl;
    std::cout << "╚═════════════════════════════════════════════╝" << std::endl;
}

std::string PFFMemoryManager::visualizeMemory() const {
    std::ostringstream oss;
    
    oss << "\n┌─────────────────────────────────┐\n";
    oss << "│  MEMORIA FÍSICA (PFF)           │\n";
    oss << "├─────────────────────────────────┤\n";
    
    int frame_num = 0;
    for (int page : resident_pages) {
        oss << "│ Frame " << frame_num++ << ": [Página " << std::setw(3) << page << "]     │\n";
    }
    
    // Mostrar frames vacíos
    for (int i = frame_num; i < current_frames; i++) {
        oss << "│ Frame " << i << ": [-------]          │\n";
    }
    
    oss << "├─────────────────────────────────┤\n";
    oss << "│ Uso: " << resident_pages.size() << "/" << current_frames 
        << " frames (" << std::fixed << std::setprecision(1)
        << (current_frames > 0 ? (resident_pages.size() * 100.0 / current_frames) : 0)
        << "%)        │\n";
    oss << "└─────────────────────────────────┘\n";
    
    return oss.str();
}

// ==================== Working Set Memory Manager ====================

WorkingSetMemoryManager::WorkingSetMemoryManager(int max, int window)
    : max_frames(max), window_size(window), current_time(0), 
      page_faults(0), page_hits(0) {}

bool WorkingSetMemoryManager::accessPage(int page_number) {
    current_time++;
    
    // Buscar si la página ya está en memoria
    auto it = std::find_if(memory.begin(), memory.end(),
        [page_number](const PageEntry& entry) {
            return entry.page_number == page_number;
        });
    
    if (it != memory.end()) {
        // Page hit
        page_hits++;
        it->last_access_time = current_time;
        it->reference_bit = true;
        return true;
    }
    
    // Page fault
    page_faults++;
    
    // Actualizar working set
    updateWorkingSet();
    
    PageEntry new_entry = {page_number, current_time, true};
    
    if (memory.size() < static_cast<size_t>(max_frames)) {
        // Hay espacio disponible
        memory.push_back(new_entry);
    } else {
        // Necesitamos reemplazar una página
        int victim_idx = findVictim();
        if (victim_idx != -1) {
            memory[victim_idx] = new_entry;
        }
    }
    
    working_set_history.push_back(getCurrentWorkingSetSize());
    
    return false;
}

void WorkingSetMemoryManager::updateWorkingSet() {
    // Remover páginas que están fuera de la ventana de tiempo
    memory.erase(
        std::remove_if(memory.begin(), memory.end(),
            [this](const PageEntry& entry) {
                return (current_time - entry.last_access_time) > window_size;
            }),
        memory.end()
    );
}

int WorkingSetMemoryManager::findVictim() {
    // Buscar la página más antigua fuera del working set
    int oldest_idx = -1;
    int oldest_time = current_time;
    
    for (size_t i = 0; i < memory.size(); i++) {
        if ((current_time - memory[i].last_access_time) > window_size) {
            if (memory[i].last_access_time < oldest_time) {
                oldest_time = memory[i].last_access_time;
                oldest_idx = i;
            }
        }
    }
    
    // Si no hay páginas fuera del working set, tomar la más antigua
    if (oldest_idx == -1 && !memory.empty()) {
        oldest_idx = 0;
        oldest_time = memory[0].last_access_time;
        for (size_t i = 1; i < memory.size(); i++) {
            if (memory[i].last_access_time < oldest_time) {
                oldest_time = memory[i].last_access_time;
                oldest_idx = i;
            }
        }
    }
    
    return oldest_idx;
}

int WorkingSetMemoryManager::getCurrentWorkingSetSize() const {
    int count = 0;
    for (const auto& entry : memory) {
        if ((current_time - entry.last_access_time) <= window_size) {
            count++;
        }
    }
    return count;
}

void WorkingSetMemoryManager::reset() {
    memory.clear();
    current_time = 0;
    page_faults = 0;
    page_hits = 0;
    working_set_history.clear();
}

PFFStats WorkingSetMemoryManager::getStats() const {
    PFFStats stats;
    stats.algorithm = "Working Set";
    stats.page_faults = page_faults;
    stats.page_hits = page_hits;
    stats.fault_rate = (page_faults + page_hits) > 0 ? 
        static_cast<double>(page_faults) / (page_faults + page_hits) : 0.0;
    stats.frame_adjustments = 0;  // No aplica para Working Set
    stats.working_set_sizes = working_set_history;
    return stats;
}

void WorkingSetMemoryManager::printStats() const {
    std::cout << "\n╔═════════════════════════════════════════════╗" << std::endl;
    std::cout << "║   ESTADÍSTICAS - Working Set                ║" << std::endl;
    std::cout << "╠═════════════════════════════════════════════╣" << std::endl;
    std::cout << "║ Fallos de página:      " << std::setw(18) << page_faults << " ║" << std::endl;
    std::cout << "║ Aciertos de página:    " << std::setw(18) << page_hits << " ║" << std::endl;
    std::cout << "║ Tasa de fallos:        " << std::setw(16) << std::fixed 
              << std::setprecision(2) << (getStats().fault_rate * 100) << "% ║" << std::endl;
    std::cout << "║ Tamaño WS actual:      " << std::setw(18) << getCurrentWorkingSetSize() << " ║" << std::endl;
    std::cout << "║ Ventana de tiempo:     " << std::setw(18) << window_size << " ║" << std::endl;
    std::cout << "║ Frames máximos:        " << std::setw(18) << max_frames << " ║" << std::endl;
    std::cout << "╚═════════════════════════════════════════════╝" << std::endl;
}

std::string WorkingSetMemoryManager::visualizeMemory() const {
    std::ostringstream oss;
    
    oss << "\n┌─────────────────────────────────────────┐\n";
    oss << "│  MEMORIA FÍSICA (Working Set)          │\n";
    oss << "├─────────────────────────────────────────┤\n";
    
    for (size_t i = 0; i < memory.size(); i++) {
        bool in_ws = (current_time - memory[i].last_access_time) <= window_size;
        oss << "│ Frame " << i << ": [Página " << std::setw(3) << memory[i].page_number 
            << "] " << (in_ws ? "✓WS" : "   ") << "      │\n";
    }
    
    oss << "├─────────────────────────────────────────┤\n";
    oss << "│ Working Set: " << getCurrentWorkingSetSize() << " páginas           │\n";
    oss << "│ Total en memoria: " << memory.size() << "/" << max_frames << " frames     │\n";
    oss << "└─────────────────────────────────────────┘\n";
    
    return oss.str();
}
