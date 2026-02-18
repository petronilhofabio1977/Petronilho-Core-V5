#include "arena.hpp"
#include <iostream>
#include <vector>
#include <thread>
#include <chrono>
#include <iomanip>
void run(int t_count) {
    const uint64_t ops_per_t = 10000000;
    petronilho::ScalableArena arena(2ULL * 1024 * 1024 * 1024);
    std::vector<std::thread> threads;
    auto start = std::chrono::steady_clock::now();
    for (int i = 0; i < t_count; ++i) {
        threads.emplace_back([&arena, ops_per_t]() {
            for (uint64_t j = 0; j < ops_per_t; ++j) {
                void* p = arena.allocate(64);
                asm volatile("" : : "g"(p) : "memory");
            }
        });
    }
    for (auto& t : threads) t.join();
    auto end = std::chrono::steady_clock::now();
    double sec = std::chrono::duration_cast<std::chrono::nanoseconds>(end-start).count() / 1e9;
    std::cout << "Threads: " << t_count << " | Mops/s: " << ((t_count * ops_per_t) / sec) / 1e6 << std::endl;
}
int main() { for (int t : {1, 2, 4, 8}) run(t); return 0; }
