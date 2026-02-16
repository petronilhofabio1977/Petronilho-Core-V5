#include <iostream>
#include <chrono>
#include <vector>
#include "core/sys/sys_core.hpp"
#include "core/sys/handle.hpp"
#include "geometric_wing/spatial_node.hpp"
#include "geometric_wing/collision.hpp"
#include "relational/hash_table.hpp"

using namespace super_core;

int main() {
    sys::boot_info();
    
    // Alocamos 256MB para aguentar o tranco
    const size_t RAM_SIZE = 256 * 1024 * 1024;
    void* buffer = nullptr;
    posix_memalign(&buffer, 64, RAM_SIZE);
    sys::G_POOL_BASE = (uint8_t*)buffer;
    sys::Arena arena(buffer, RAM_SIZE);

    const int TOTAL_OBJECTS = 1000000;
    relational::HashTable<uint32_t, geometric::BoundingBox> scene(arena, TOTAL_OBJECTS * 2);

    std::cout << "[STRESS] Alocando e Indexando 1 milhao de objetos..." << std::endl;
    
    for(int i = 0; i < TOTAL_OBJECTS; ++i) {
        auto h = arena.allocate<geometric::BoundingBox>();
        h->min_p = {(float)i, (float)i}; 
        h->max_p = {(float)i + 5, (float)i + 5};
        scene.insert(i, h);
    }

    std::cout << "[STRESS] Iniciando processamento de colisoes massivo..." << std::endl;
    
    auto start = std::chrono::high_resolution_clock::now();
    
    int collisions_found = 0;
    // Testamos cada objeto com o seu vizinho imediato (i e i+1)
    for(int i = 0; i < TOTAL_OBJECTS - 1; ++i) {
        auto obj1 = scene.get(i);
        auto obj2 = scene.get(i + 1);
        
        if (geometric::check_collision(*obj1, *obj2)) {
            collisions_found++;
        }
    }

    auto end = std::chrono::high_resolution_clock::now();
    auto diff = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);

    std::cout << "==============================================" << std::endl;
    std::cout << "RESULTADO DO TESTE DE ESTRESSE:" << std::endl;
    std::cout << "Objetos Processados: " << TOTAL_OBJECTS << std::endl;
    std::cout << "Colisoes Detectadas: " << collisions_found << std::endl;
    std::cout << "Tempo Total: " << diff.count() << " ms" << std::endl;
    std::cout << "Latencia por par: " << (double)diff.count() * 1000000 / TOTAL_OBJECTS << " ns" << std::endl;
    std::cout << "==============================================" << std::endl;

    free(buffer);
    return 0;
}
