#include "arena.hpp"
#include <iostream>
#include <chrono>
#include <iomanip>
int main() {
    using namespace std::chrono;
    petronilho::ScalableArena arena(1024ULL * 1024 * 1024);
    auto start = steady_clock::now();
    uint64_t ops = 0;
    while (steady_clock::now() - start < seconds(2)) {
        for (int i = 0; i < 1000; ++i) {
            void* p = arena.allocate(64);
            if (!p) { arena.reset(); p = arena.allocate(64); }
            asm volatile("" : : "g"(p) : "memory");
            ops++;
        }
    }
    auto end = steady_clock::now();
    double duration = duration_cast<nanoseconds>(end - start).count() / 1e9;
    std::cout << "Throughput: " << std::fixed << std::setprecision(2) << (ops / duration) / 1e6 << " Mops/sec" << std::endl;
    return 0;
}
