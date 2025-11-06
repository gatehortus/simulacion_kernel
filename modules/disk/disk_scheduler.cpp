#include "disk_scheduler.h"
#include <iostream>
#include <iomanip>
#include <sstream>

DiskScheduler::DiskScheduler(int cylinders, int initial_head) 
    : num_cylinders(cylinders), current_head(initial_head) {}

void DiskScheduler::addRequest(int cylinder, int arrival_time, int pid) {
    if (cylinder >= 0 && cylinder < num_cylinders) {
        request_queue.push_back({cylinder, arrival_time, pid});
    }
}

void DiskScheduler::clearQueue() {
    request_queue.clear();
}

// FCFS - First Come First Served
DiskStats DiskScheduler::scheduleFCFS() {
    DiskStats stats;
    stats.algorithm = "FCFS";
    stats.total_head_movement = 0;
    
    int head = current_head;
    stats.sequence.push_back(head);
    
    for (const auto& req : request_queue) {
        stats.total_head_movement += abs(head - req.cylinder);
        head = req.cylinder;
        stats.sequence.push_back(head);
    }
    
    stats.avg_wait_time = request_queue.empty() ? 0 : 
        static_cast<double>(stats.total_head_movement) / request_queue.size();
    
    return stats;
}

// SSTF - Shortest Seek Time First
DiskStats DiskScheduler::scheduleSSTF() {
    DiskStats stats;
    stats.algorithm = "SSTF";
    stats.total_head_movement = 0;
    
    std::vector<DiskRequest> remaining = request_queue;
    int head = current_head;
    stats.sequence.push_back(head);
    
    while (!remaining.empty()) {
        // Encontrar la solicitud más cercana
        int closest_idx = 0;
        int min_distance = abs(remaining[0].cylinder - head);
        
        for (size_t i = 1; i < remaining.size(); i++) {
            int distance = abs(remaining[i].cylinder - head);
            if (distance < min_distance) {
                min_distance = distance;
                closest_idx = i;
            }
        }
        
        stats.total_head_movement += min_distance;
        head = remaining[closest_idx].cylinder;
        stats.sequence.push_back(head);
        remaining.erase(remaining.begin() + closest_idx);
    }
    
    stats.avg_wait_time = request_queue.empty() ? 0 : 
        static_cast<double>(stats.total_head_movement) / request_queue.size();
    
    return stats;
}

// SCAN (Elevator Algorithm)
DiskStats DiskScheduler::scheduleSCAN(bool direction_up) {
    DiskStats stats;
    stats.algorithm = "SCAN";
    stats.total_head_movement = 0;
    
    std::vector<DiskRequest> remaining = request_queue;
    std::sort(remaining.begin(), remaining.end(), 
              [](const DiskRequest& a, const DiskRequest& b) {
                  return a.cylinder < b.cylinder;
              });
    
    int head = current_head;
    stats.sequence.push_back(head);
    
    // Separar solicitudes en dos grupos
    std::vector<DiskRequest> left, right;
    for (const auto& req : remaining) {
        if (req.cylinder < head) {
            left.push_back(req);
        } else {
            right.push_back(req);
        }
    }
    
    if (direction_up) {
        // Ir hacia arriba primero
        for (const auto& req : right) {
            stats.total_head_movement += abs(head - req.cylinder);
            head = req.cylinder;
            stats.sequence.push_back(head);
        }
        
        // Llegar al final
        if (!right.empty()) {
            stats.total_head_movement += abs(head - (num_cylinders - 1));
            head = num_cylinders - 1;
            stats.sequence.push_back(head);
        }
        
        // Volver y atender las de abajo
        for (auto it = left.rbegin(); it != left.rend(); ++it) {
            stats.total_head_movement += abs(head - it->cylinder);
            head = it->cylinder;
            stats.sequence.push_back(head);
        }
    } else {
        // Ir hacia abajo primero
        for (auto it = left.rbegin(); it != left.rend(); ++it) {
            stats.total_head_movement += abs(head - it->cylinder);
            head = it->cylinder;
            stats.sequence.push_back(head);
        }
        
        // Llegar al inicio
        if (!left.empty()) {
            stats.total_head_movement += abs(head - 0);
            head = 0;
            stats.sequence.push_back(head);
        }
        
        // Volver y atender las de arriba
        for (const auto& req : right) {
            stats.total_head_movement += abs(head - req.cylinder);
            head = req.cylinder;
            stats.sequence.push_back(head);
        }
    }
    
    stats.avg_wait_time = request_queue.empty() ? 0 : 
        static_cast<double>(stats.total_head_movement) / request_queue.size();
    
    return stats;
}

// C-SCAN (Circular SCAN)
DiskStats DiskScheduler::scheduleCSCAN() {
    DiskStats stats;
    stats.algorithm = "C-SCAN";
    stats.total_head_movement = 0;
    
    std::vector<DiskRequest> remaining = request_queue;
    std::sort(remaining.begin(), remaining.end(), 
              [](const DiskRequest& a, const DiskRequest& b) {
                  return a.cylinder < b.cylinder;
              });
    
    int head = current_head;
    stats.sequence.push_back(head);
    
    std::vector<DiskRequest> left, right;
    for (const auto& req : remaining) {
        if (req.cylinder < head) {
            left.push_back(req);
        } else {
            right.push_back(req);
        }
    }
    
    // Ir hacia arriba
    for (const auto& req : right) {
        stats.total_head_movement += abs(head - req.cylinder);
        head = req.cylinder;
        stats.sequence.push_back(head);
    }
    
    // Llegar al final
    if (!right.empty() && head != num_cylinders - 1) {
        stats.total_head_movement += abs(head - (num_cylinders - 1));
        head = num_cylinders - 1;
        stats.sequence.push_back(head);
    }
    
    // Saltar al inicio
    if (!left.empty()) {
        stats.total_head_movement += abs(head - 0);
        head = 0;
        stats.sequence.push_back(head);
        
        // Atender solicitudes desde el inicio
        for (const auto& req : left) {
            stats.total_head_movement += abs(head - req.cylinder);
            head = req.cylinder;
            stats.sequence.push_back(head);
        }
    }
    
    stats.avg_wait_time = request_queue.empty() ? 0 : 
        static_cast<double>(stats.total_head_movement) / request_queue.size();
    
    return stats;
}

// LOOK (like SCAN but doesn't go to the end)
DiskStats DiskScheduler::scheduleLOOK(bool direction_up) {
    DiskStats stats;
    stats.algorithm = "LOOK";
    stats.total_head_movement = 0;
    
    std::vector<DiskRequest> remaining = request_queue;
    std::sort(remaining.begin(), remaining.end(), 
              [](const DiskRequest& a, const DiskRequest& b) {
                  return a.cylinder < b.cylinder;
              });
    
    int head = current_head;
    stats.sequence.push_back(head);
    
    std::vector<DiskRequest> left, right;
    for (const auto& req : remaining) {
        if (req.cylinder < head) {
            left.push_back(req);
        } else {
            right.push_back(req);
        }
    }
    
    if (direction_up) {
        for (const auto& req : right) {
            stats.total_head_movement += abs(head - req.cylinder);
            head = req.cylinder;
            stats.sequence.push_back(head);
        }
        
        for (auto it = left.rbegin(); it != left.rend(); ++it) {
            stats.total_head_movement += abs(head - it->cylinder);
            head = it->cylinder;
            stats.sequence.push_back(head);
        }
    } else {
        for (auto it = left.rbegin(); it != left.rend(); ++it) {
            stats.total_head_movement += abs(head - it->cylinder);
            head = it->cylinder;
            stats.sequence.push_back(head);
        }
        
        for (const auto& req : right) {
            stats.total_head_movement += abs(head - req.cylinder);
            head = req.cylinder;
            stats.sequence.push_back(head);
        }
    }
    
    stats.avg_wait_time = request_queue.empty() ? 0 : 
        static_cast<double>(stats.total_head_movement) / request_queue.size();
    
    return stats;
}

void DiskScheduler::printStats(const DiskStats& stats) {
    std::cout << "\n=== Estadísticas de " << stats.algorithm << " ===" << std::endl;
    std::cout << "Movimiento total del cabezal: " << stats.total_head_movement << " cilindros" << std::endl;
    std::cout << "Tiempo promedio de espera: " << std::fixed << std::setprecision(2) 
              << stats.avg_wait_time << std::endl;
    std::cout << "Secuencia: ";
    for (size_t i = 0; i < stats.sequence.size(); i++) {
        std::cout << stats.sequence[i];
        if (i < stats.sequence.size() - 1) std::cout << " -> ";
    }
    std::cout << std::endl;
}

std::string DiskScheduler::visualizeDisk(const DiskStats& stats) {
    std::ostringstream oss;
    const int width = 60;
    
    oss << "\n╔════════════════════════════════════════════════════════════╗\n";
    oss << "║ Visualización de Movimiento del Disco - " << std::setw(20) << std::left 
        << stats.algorithm << "║\n";
    oss << "╠════════════════════════════════════════════════════════════╣\n";
    
    for (size_t i = 0; i < stats.sequence.size() - 1; i++) {
        int from = stats.sequence[i];
        int to = stats.sequence[i + 1];
        int distance = abs(to - from);
        
        oss << "║ " << std::setw(3) << from << " -> " << std::setw(3) << to 
            << " (dist: " << std::setw(3) << distance << ")   ";
        
        // Barra visual
        int bar_length = std::min(30, (distance * 30) / num_cylinders);
        oss << "[";
        for (int j = 0; j < bar_length; j++) oss << "█";
        for (int j = bar_length; j < 30; j++) oss << " ";
        oss << "] ║\n";
    }
    
    oss << "╠════════════════════════════════════════════════════════════╣\n";
    oss << "║ Total: " << std::setw(3) << stats.total_head_movement 
        << " cilindros | Promedio: " << std::fixed << std::setprecision(2) 
        << std::setw(6) << stats.avg_wait_time << "                   ║\n";
    oss << "╚════════════════════════════════════════════════════════════╝\n";
    
    return oss.str();
}
