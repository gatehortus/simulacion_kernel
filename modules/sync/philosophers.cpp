#include "philosophers.h"
#include <iostream>
#include <iomanip>
#include <sstream>
#include <random>

DiningPhilosophers::DiningPhilosophers(int num) 
    : num_philosophers(num), running(false) {
    forks.resize(num);
    states.resize(num, PhilosopherState::THINKING);
    stats.resize(num);
    
    for (int i = 0; i < num; i++) {
        stats[i] = {i, 0, 0, 0, 0, PhilosopherState::THINKING};
    }
}

DiningPhilosophers::~DiningPhilosophers() {
    stop();
}

void DiningPhilosophers::start(int meals_per_philosopher, bool use_safe_version) {
    if (running) return;
    
    running = true;
    threads.clear();
    
    std::cout << "\n🍽️  Iniciando Cena de los Filósofos (" 
              << (use_safe_version ? "Versión SEGURA" : "Versión BÁSICA") 
              << ")\n" << std::endl;
    
    for (int i = 0; i < num_philosophers; i++) {
        threads.emplace_back(&DiningPhilosophers::philosopher_routine, this, i, meals_per_philosopher);
    }
}

void DiningPhilosophers::stop() {
    running = false;
    cv.notify_all();
    
    for (auto& t : threads) {
        if (t.joinable()) {
            t.join();
        }
    }
    threads.clear();
}

void DiningPhilosophers::philosopher_routine(int id, int meals_to_eat) {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> think_time(100, 500);
    std::uniform_int_distribution<> eat_time(100, 300);
    
    for (int meal = 0; meal < meals_to_eat && running; meal++) {
        // Pensar
        think(id);
        auto think_duration = think_time(gen);
        std::this_thread::sleep_for(std::chrono::milliseconds(think_duration));
        stats[id].total_thinking_time += think_duration;
        
        // Intentar comer
        {
            std::lock_guard<std::mutex> lock(state_mutex);
            states[id] = PhilosopherState::HUNGRY;
            stats[id].times_waited++;
        }
        
        pickup_forks_safe(id);
        
        // Comer
        eat(id);
        auto eat_duration = eat_time(gen);
        std::this_thread::sleep_for(std::chrono::milliseconds(eat_duration));
        stats[id].total_eating_time += eat_duration;
        stats[id].times_eaten++;
        
        putdown_forks_safe(id);
    }
    
    {
        std::lock_guard<std::mutex> lock(state_mutex);
        states[id] = PhilosopherState::THINKING;
    }
}

void DiningPhilosophers::think(int id) {
    std::lock_guard<std::mutex> lock(output_mutex);
    std::cout << "🤔 Filósofo " << id << " está pensando..." << std::endl;
}

void DiningPhilosophers::eat(int id) {
    std::lock_guard<std::mutex> lock(output_mutex);
    std::cout << "🍝 Filósofo " << id << " está comiendo (comida #" 
              << stats[id].times_eaten + 1 << ")" << std::endl;
}

// Versión segura: evita deadlock usando orden de adquisición
void DiningPhilosophers::pickup_forks_safe(int id) {
    int left_fork = id;
    int right_fork = (id + 1) % num_philosophers;
    
    // Solución: siempre tomar el tenedor de menor índice primero
    int first_fork = std::min(left_fork, right_fork);
    int second_fork = std::max(left_fork, right_fork);
    
    forks[first_fork].lock();
    forks[second_fork].lock();
    
    {
        std::lock_guard<std::mutex> lock(state_mutex);
        states[id] = PhilosopherState::EATING;
    }
}

void DiningPhilosophers::putdown_forks_safe(int id) {
    int left_fork = id;
    int right_fork = (id + 1) % num_philosophers;
    
    forks[left_fork].unlock();
    forks[right_fork].unlock();
    
    {
        std::lock_guard<std::mutex> lock(state_mutex);
        states[id] = PhilosopherState::THINKING;
    }
}

// Versión básica (puede causar deadlock)
void DiningPhilosophers::pickup_forks(int id) {
    int left_fork = id;
    int right_fork = (id + 1) % num_philosophers;
    
    forks[left_fork].lock();
    forks[right_fork].lock();
}

void DiningPhilosophers::putdown_forks(int id) {
    int left_fork = id;
    int right_fork = (id + 1) % num_philosophers;
    
    forks[left_fork].unlock();
    forks[right_fork].unlock();
}

std::vector<PhilosopherStats> DiningPhilosophers::getStats() const {
    return stats;
}

void DiningPhilosophers::printStats() const {
    std::cout << "\n╔═══════════════════════════════════════════════════════════════╗" << std::endl;
    std::cout << "║           ESTADÍSTICAS DE LA CENA DE LOS FILÓSOFOS           ║" << std::endl;
    std::cout << "╠═══╦═══════════╦═══════════╦═══════════╦═══════════╦═════════╣" << std::endl;
    std::cout << "║ID ║ Estado    ║ Comidas   ║ Esperas   ║ T.Pensar  ║ T.Comer ║" << std::endl;
    std::cout << "╠═══╬═══════════╬═══════════╬═══════════╬═══════════╬═════════╣" << std::endl;
    
    for (const auto& s : stats) {
        std::string state_str;
        switch (states[s.id]) {
            case PhilosopherState::THINKING: state_str = "Pensando"; break;
            case PhilosopherState::HUNGRY:   state_str = "Hambriento"; break;
            case PhilosopherState::EATING:   state_str = "Comiendo"; break;
        }
        
        std::cout << "║ " << s.id << " ║ " 
                  << std::setw(9) << std::left << state_str << " ║ "
                  << std::setw(9) << s.times_eaten << " ║ "
                  << std::setw(9) << s.times_waited << " ║ "
                  << std::setw(9) << s.total_thinking_time << " ║ "
                  << std::setw(7) << s.total_eating_time << " ║" << std::endl;
    }
    
    std::cout << "╚═══╩═══════════╩═══════════╩═══════════╩═══════════╩═════════╝" << std::endl;
}

std::string DiningPhilosophers::visualizeTable() const {
    std::ostringstream oss;
    
    oss << "\n     MESA DE LOS FILÓSOFOS\n";
    oss << "     (🍴 = tenedor)\n\n";
    oss << "          🧑 F0\n";
    oss << "      🍴         🍴\n";
    oss << "   🧑 F4        F1 🧑\n";
    oss << "      🍴       🍴\n";
    oss << "        🧑 F3 🧑\n";
    oss << "           F2\n\n";
    
    oss << "Estados actuales:\n";
    for (size_t i = 0; i < states.size(); i++) {
        oss << "F" << i << ": ";
        switch (states[i]) {
            case PhilosopherState::THINKING: oss << "🤔 Pensando"; break;
            case PhilosopherState::HUNGRY:   oss << "😋 Hambriento"; break;
            case PhilosopherState::EATING:   oss << "🍝 Comiendo"; break;
        }
        oss << "\n";
    }
    
    return oss.str();
}
