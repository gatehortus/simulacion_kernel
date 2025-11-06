#include "heap_allocator.h"
#include <iostream>
#include <iomanip>
#include <sstream>
#include <algorithm>
#include <cstring>

// ==================== Buddy System Allocator ====================

BuddyAllocator::BuddyAllocator(size_t size) {
    // Asegurar que el tamaño sea potencia de 2
    total_size = 1;
    max_level = 0;
    while (total_size < size) {
        total_size *= 2;
        max_level++;
    }
    
    free_lists.resize(max_level + 1);
    
    // Crear el bloque inicial
    Block* initial = new Block{total_size, true, 0, max_level};
    free_lists[max_level].push_back(initial);
    
    stats = {total_size, 0, total_size, 0.0, 0.0, 0, 0, 0, "Buddy System"};
}

BuddyAllocator::~BuddyAllocator() {
    for (auto& list : free_lists) {
        for (auto block : list) {
            delete block;
        }
    }
    for (auto& pair : allocated_blocks) {
        delete pair.second;
    }
}

int BuddyAllocator::calculateLevel(size_t size) {
    // Encontrar el nivel más pequeño que pueda contener el tamaño
    size_t block_size = 1;
    int level = 0;
    
    while (block_size < size && level < max_level) {
        block_size *= 2;
        level++;
    }
    
    return level;
}

size_t BuddyAllocator::getLevelSize(int level) {
    return total_size >> (max_level - level);
}

Block* BuddyAllocator::split(Block* block, int target_level) {
    if (block->level == target_level) {
        return block;
    }
    
    // Dividir el bloque
    int new_level = block->level - 1;
    size_t new_size = getLevelSize(new_level);
    
    Block* buddy = new Block{new_size, true, block->address + new_size, new_level};
    free_lists[new_level].push_back(buddy);
    
    block->size = new_size;
    block->level = new_level;
    
    return split(block, target_level);
}

Block* BuddyAllocator::findBuddy(Block* block) {
    size_t buddy_address;
    if ((block->address / block->size) % 2 == 0) {
        buddy_address = block->address + block->size;
    } else {
        buddy_address = block->address - block->size;
    }
    
    // Buscar el buddy en la lista de libres
    for (auto it = free_lists[block->level].begin(); 
         it != free_lists[block->level].end(); ++it) {
        if ((*it)->address == buddy_address) {
            Block* buddy = *it;
            free_lists[block->level].erase(it);
            return buddy;
        }
    }
    
    return nullptr;
}

void BuddyAllocator::coalesce(Block* block) {
    while (block->level < max_level) {
        Block* buddy = findBuddy(block);
        if (!buddy) break;
        
        // Fusionar con el buddy
        if (block->address > buddy->address) {
            std::swap(block, buddy);
        }
        
        block->size *= 2;
        block->level++;
        
        delete buddy;
        stats.coalesces++;
    }
    
    free_lists[block->level].push_back(block);
}

void* BuddyAllocator::allocate(size_t size) {
    if (size == 0 || size > total_size) return nullptr;
    
    int level = calculateLevel(size);
    
    // Buscar un bloque libre del nivel apropiado
    Block* block = nullptr;
    for (int l = level; l <= max_level; l++) {
        if (!free_lists[l].empty()) {
            block = free_lists[l].back();
            free_lists[l].pop_back();
            
            // Dividir si es necesario
            if (l > level) {
                block = split(block, level);
            }
            break;
        }
    }
    
    if (!block) return nullptr;  // No hay memoria disponible
    
    block->is_free = false;
    allocated_blocks[block->address] = block;
    
    stats.allocations++;
    stats.used_memory += block->size;
    stats.free_memory -= block->size;
    
    // Fragmentación interna
    if (size < block->size) {
        stats.fragmentation_internal += (block->size - size);
    }
    
    updateFragmentation();
    
    return reinterpret_cast<void*>(block->address);
}

void BuddyAllocator::deallocate(void* ptr) {
    if (!ptr) return;
    
    size_t address = reinterpret_cast<size_t>(ptr);
    auto it = allocated_blocks.find(address);
    
    if (it == allocated_blocks.end()) return;
    
    Block* block = it->second;
    allocated_blocks.erase(it);
    
    block->is_free = true;
    stats.deallocations++;
    stats.used_memory -= block->size;
    stats.free_memory += block->size;
    
    coalesce(block);
    updateFragmentation();
}

void BuddyAllocator::updateFragmentation() {
    size_t largest_free = 0;
    for (const auto& list : free_lists) {
        for (const auto& block : list) {
            if (block->size > largest_free) {
                largest_free = block->size;
            }
        }
    }
    
    stats.fragmentation_external = stats.free_memory > 0 ? 
        1.0 - (static_cast<double>(largest_free) / stats.free_memory) : 0.0;
}

void BuddyAllocator::reset() {
    for (auto& list : free_lists) {
        for (auto block : list) {
            delete block;
        }
        list.clear();
    }
    
    for (auto& pair : allocated_blocks) {
        delete pair.second;
    }
    allocated_blocks.clear();
    
    Block* initial = new Block{total_size, true, 0, max_level};
    free_lists[max_level].push_back(initial);
    
    stats = {total_size, 0, total_size, 0.0, 0.0, 0, 0, 0, "Buddy System"};
}

void BuddyAllocator::printStats() const {
    std::cout << "\n╔═══════════════════════════════════════════════════════╗" << std::endl;
    std::cout << "║        ESTADÍSTICAS - BUDDY SYSTEM ALLOCATOR          ║" << std::endl;
    std::cout << "╠═══════════════════════════════════════════════════════╣" << std::endl;
    std::cout << "║ Memoria total:           " << std::setw(24) << stats.total_memory << " bytes ║" << std::endl;
    std::cout << "║ Memoria usada:           " << std::setw(24) << stats.used_memory << " bytes ║" << std::endl;
    std::cout << "║ Memoria libre:           " << std::setw(24) << stats.free_memory << " bytes ║" << std::endl;
    std::cout << "║ Fragmentación externa:   " << std::setw(23) << std::fixed 
              << std::setprecision(2) << (stats.fragmentation_external * 100) << "% ║" << std::endl;
    std::cout << "║ Fragmentación interna:   " << std::setw(24) 
              << stats.fragmentation_internal << " bytes ║" << std::endl;
    std::cout << "║ Asignaciones:            " << std::setw(27) << stats.allocations << " ║" << std::endl;
    std::cout << "║ Liberaciones:            " << std::setw(27) << stats.deallocations << " ║" << std::endl;
    std::cout << "║ Coalescencias:           " << std::setw(27) << stats.coalesces << " ║" << std::endl;
    std::cout << "╚═══════════════════════════════════════════════════════╝" << std::endl;
}

std::string BuddyAllocator::visualizeHeap() const {
    std::ostringstream oss;
    
    oss << "\n┌───────────────────────────────────────────────┐\n";
    oss << "│       VISUALIZACIÓN DEL HEAP (Buddy)         │\n";
    oss << "├───────────────────────────────────────────────┤\n";
    
    for (int level = max_level; level >= 0; level--) {
        size_t block_size = getLevelSize(level);
        oss << "│ Nivel " << level << " (" << block_size << " bytes):\n";
        
        if (free_lists[level].empty()) {
            oss << "│   [Vacío]\n";
        } else {
            for (const auto& block : free_lists[level]) {
                oss << "│   Libre @ " << block->address << "\n";
            }
        }
    }
    
    oss << "├───────────────────────────────────────────────┤\n";
    oss << "│ Bloques asignados: " << allocated_blocks.size() << "\n";
    
    int count = 0;
    for (const auto& pair : allocated_blocks) {
        if (count++ < 5) {  // Mostrar solo los primeros 5
            oss << "│   @ " << pair.first << " (" << pair.second->size << " bytes)\n";
        }
    }
    if (allocated_blocks.size() > 5) {
        oss << "│   ... y " << (allocated_blocks.size() - 5) << " más\n";
    }
    
    oss << "└───────────────────────────────────────────────┘\n";
    
    return oss.str();
}

// ==================== Segregated Allocator ====================

SegregatedAllocator::SegregatedAllocator(size_t size) : total_size(size) {
    // Definir clases de tamaño: 8, 16, 32, 64, 128, 256, 512 bytes
    std::vector<size_t> sizes = {8, 16, 32, 64, 128, 256, 512};
    
    for (size_t s : sizes) {
        SizeClass sc;
        sc.block_size = s;
        sc.total_blocks = 0;
        size_classes.push_back(sc);
    }
    
    stats = {total_size, 0, total_size, 0.0, 0.0, 0, 0, 0, "Segregated Free List"};
}

SegregatedAllocator::~SegregatedAllocator() {
    for (auto& sc : size_classes) {
        for (auto block : sc.free_blocks) {
            delete block;
        }
    }
    for (auto& pair : allocated_blocks) {
        delete pair.second;
    }
}

int SegregatedAllocator::findSizeClass(size_t size) {
    for (size_t i = 0; i < size_classes.size(); i++) {
        if (size <= size_classes[i].block_size) {
            return i;
        }
    }
    return -1;  // Tamaño demasiado grande
}

Block* SegregatedAllocator::allocateNewSlab(int class_index) {
    if (stats.used_memory + size_classes[class_index].block_size > total_size) {
        return nullptr;
    }
    
    size_t address = stats.used_memory;
    Block* block = new Block{
        size_classes[class_index].block_size, 
        true, 
        address, 
        class_index
    };
    
    size_classes[class_index].total_blocks++;
    return block;
}

void* SegregatedAllocator::allocate(size_t size) {
    if (size == 0) return nullptr;
    
    int class_idx = findSizeClass(size);
    if (class_idx == -1) return nullptr;
    
    Block* block;
    
    if (size_classes[class_idx].free_blocks.empty()) {
        block = allocateNewSlab(class_idx);
        if (!block) return nullptr;
    } else {
        block = size_classes[class_idx].free_blocks.back();
        size_classes[class_idx].free_blocks.pop_back();
    }
    
    block->is_free = false;
    allocated_blocks[block->address] = block;
    
    stats.allocations++;
    stats.used_memory += block->size;
    stats.free_memory = total_size - stats.used_memory;
    
    if (size < block->size) {
        stats.fragmentation_internal += (block->size - size);
    }
    
    updateFragmentation();
    
    return reinterpret_cast<void*>(block->address);
}

void SegregatedAllocator::deallocate(void* ptr) {
    if (!ptr) return;
    
    size_t address = reinterpret_cast<size_t>(ptr);
    auto it = allocated_blocks.find(address);
    
    if (it == allocated_blocks.end()) return;
    
    Block* block = it->second;
    allocated_blocks.erase(it);
    
    block->is_free = true;
    size_classes[block->level].free_blocks.push_back(block);
    
    stats.deallocations++;
    stats.used_memory -= block->size;
    stats.free_memory = total_size - stats.used_memory;
    
    updateFragmentation();
}

void SegregatedAllocator::updateFragmentation() {
    // Calcular fragmentación externa
    size_t total_free = 0;
    size_t largest_free = 0;
    
    for (const auto& sc : size_classes) {
        size_t class_free = sc.free_blocks.size() * sc.block_size;
        total_free += class_free;
        if (sc.block_size > largest_free && !sc.free_blocks.empty()) {
            largest_free = sc.block_size;
        }
    }
    
    stats.fragmentation_external = total_free > 0 ? 
        1.0 - (static_cast<double>(largest_free) / total_free) : 0.0;
}

void SegregatedAllocator::reset() {
    for (auto& sc : size_classes) {
        for (auto block : sc.free_blocks) {
            delete block;
        }
        sc.free_blocks.clear();
        sc.total_blocks = 0;
    }
    
    for (auto& pair : allocated_blocks) {
        delete pair.second;
    }
    allocated_blocks.clear();
    
    stats = {total_size, 0, total_size, 0.0, 0.0, 0, 0, 0, "Segregated Free List"};
}

void SegregatedAllocator::printStats() const {
    std::cout << "\n╔═══════════════════════════════════════════════════════╗" << std::endl;
    std::cout << "║    ESTADÍSTICAS - SEGREGATED FREE LIST ALLOCATOR      ║" << std::endl;
    std::cout << "╠═══════════════════════════════════════════════════════╣" << std::endl;
    std::cout << "║ Memoria total:           " << std::setw(24) << stats.total_memory << " bytes ║" << std::endl;
    std::cout << "║ Memoria usada:           " << std::setw(24) << stats.used_memory << " bytes ║" << std::endl;
    std::cout << "║ Memoria libre:           " << std::setw(24) << stats.free_memory << " bytes ║" << std::endl;
    std::cout << "║ Fragmentación externa:   " << std::setw(23) << std::fixed 
              << std::setprecision(2) << (stats.fragmentation_external * 100) << "% ║" << std::endl;
    std::cout << "║ Fragmentación interna:   " << std::setw(24) 
              << stats.fragmentation_internal << " bytes ║" << std::endl;
    std::cout << "║ Asignaciones:            " << std::setw(27) << stats.allocations << " ║" << std::endl;
    std::cout << "║ Liberaciones:            " << std::setw(27) << stats.deallocations << " ║" << std::endl;
    std::cout << "╚═══════════════════════════════════════════════════════╝" << std::endl;
}

std::string SegregatedAllocator::visualizeHeap() const {
    std::ostringstream oss;
    
    oss << "\n┌───────────────────────────────────────────────┐\n";
    oss << "│    VISUALIZACIÓN DEL HEAP (Segregated)       │\n";
    oss << "├───────────────────────────────────────────────┤\n";
    
    for (size_t i = 0; i < size_classes.size(); i++) {
        const auto& sc = size_classes[i];
        oss << "│ Clase " << sc.block_size << " bytes:\n";
        oss << "│   Total: " << sc.total_blocks 
            << " | Libres: " << sc.free_blocks.size() << "\n";
    }
    
    oss << "├───────────────────────────────────────────────┤\n";
    oss << "│ Bloques asignados: " << allocated_blocks.size() << "\n";
    oss << "└───────────────────────────────────────────────┘\n";
    
    return oss.str();
}
