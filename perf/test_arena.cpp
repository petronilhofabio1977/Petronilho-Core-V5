#include <iostream>
#include <chrono>
#include <stdlib.h> 
#include "core/sys/arena.hpp"

using namespace super_core;

int main() {
    const size_t GIGABYTE = 1024LL * 1024LL * 1024LL;
    void* raw = nullptr;
    posix_memalign(&raw, 64, GIGABYTE);
    sys::Arena arena(raw, GIGABYTE);
    
    struct Node { uint64_t d[8]; };
    const int iter = 1000000;

    auto start = std::chrono::high_resolution_clock::now();
    
    uintptr_t sum = 0;
    for(int i = 0; i < iter; ++i) {
        auto h = arena.allocate<Node>();
        sum += h.index; // Força dependência de dados
    }

    auto end = std::chrono::high_resolution_clock::now();
    
    // Barreira contra otimização
    if(sum == 0) std::cout << " "; 

    auto diff = std::chrono::duration_cast<std::chrono::nanoseconds>(end - start);
    std::cout << "Latencia Real: " << (double)diff.count() / iter << " ns/elem" << std::endl;
    
    free(raw);
    return 0;
}
