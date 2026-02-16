#include <iostream>
#include <vector>
#include <thread>
#include <chrono>
#include <x86intrin.h> 
#include "core/sys/arena_atomic.hpp"
#include "core/sys/handle.hpp"

using namespace super_core;

void worker(sys::ScalableArena& arena, int count, unsigned long long* total_cycles) {
    unsigned long long start = __rdtsc();
    for(int i = 0; i < count; ++i) {
        // Agora usando a lógica de Chunk Local
        arena.allocate<uint64_t>();
    }
    *total_cycles = __rdtsc() - start;
}

int main() {
    const size_t SIZE = 128 * 1024 * 1024;
    void* buf = nullptr;
    posix_memalign(&buf, 64, SIZE);
    
    // Sincronizado com o novo nome da classe
    sys::ScalableArena arena(buf, SIZE);

    unsigned long long thread_cycles[4] = {0};
    std::vector<std::thread> threads;

    std::cout << "[SENIOR] Iniciando Telemetria RDTSC (4 threads - Scalable Mode)..." << std::endl;

    for(int i = 0; i < 4; ++i) {
        threads.emplace_back(worker, std::ref(arena), 250000, &thread_cycles[i]);
    }
    for(auto& t : threads) t.join();

    unsigned long long total_cycles = 0;
    for(int i=0; i<4; ++i) total_cycles += thread_cycles[i];

    std::cout << "==============================================" << std::endl;
    std::cout << "Ciclos medios por alocacao (Scalable): " << total_cycles / 1000000 << " cycles" << std::endl;
    std::cout << "==============================================" << std::endl;

    free(buf);
    return 0;
}
