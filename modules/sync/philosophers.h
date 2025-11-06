#ifndef PHILOSOPHERS_H
#define PHILOSOPHERS_H

#include <vector>
#include <mutex>
#include <thread>
#include <condition_variable>
#include <chrono>
#include <atomic>
#include <string>

enum class PhilosopherState {
    THINKING,
    HUNGRY,
    EATING
};

struct PhilosopherStats {
    int id;
    int times_eaten;
    int times_waited;
    int total_thinking_time;
    int total_eating_time;
    PhilosopherState state;
};

class DiningPhilosophers {
private:
    int num_philosophers;
    std::vector<std::mutex> forks;
    std::vector<PhilosopherState> states;
    std::vector<PhilosopherStats> stats;
    std::vector<std::thread> threads;
    std::mutex output_mutex;
    std::mutex state_mutex;
    std::atomic<bool> running;
    
    // Para solución sin deadlock
    std::condition_variable cv;
    
    void philosopher_routine(int id, int meals_to_eat);
    void think(int id);
    void eat(int id);
    void pickup_forks(int id);
    void putdown_forks(int id);
    
    // Solución alternativa: con monitor
    void pickup_forks_safe(int id);
    void putdown_forks_safe(int id);
    void test(int id);
    
public:
    DiningPhilosophers(int num = 5);
    ~DiningPhilosophers();
    
    void start(int meals_per_philosopher = 5, bool use_safe_version = true);
    void stop();
    std::vector<PhilosopherStats> getStats() const;
    void printStats() const;
    std::string visualizeTable() const;
};

#endif // PHILOSOPHERS_H
