#include <iostream>
#include <stdlib.h>
#include "core/sys/sys_core.hpp"
#include "core/sys/handle.hpp"
#include "priority_wing/binary_heap.hpp"
#include "geometric_wing/spatial_node.hpp"

using namespace super_core;

int main() {
    sys::boot_info();
    
    const size_t RAM_SIZE = 64 * 1024 * 1024;
    void* buffer = nullptr;
    posix_memalign(&buffer, 64, RAM_SIZE);
    
    // VITAL: Sincroniza a base global com o buffer alocado
    sys::G_POOL_BASE = (uint8_t*)buffer;
    
    sys::Arena arena(buffer, RAM_SIZE);
    priority::BinaryHeap<geometric::Point2D> heap(arena, 100);

    std::cout << "[GEOM] Inserindo pontos..." << std::endl;
    float vals[] = {10.0f, 2.0f, 15.0f, 1.0f, 5.0f};
    
    for(float v : vals) {
        auto h = arena.allocate<geometric::Point2D>();
        h->x = v; h->y = v;
        heap.push(h);
    }

    std::cout << "[GEOM] Extraindo ordenado:" << std::endl;
    while(!heap.empty()) {
        auto p = heap.pop();
        if(!p.is_null())
            std::cout << " -> Distancia: " << p->x << std::endl;
    }
    
    free(buffer);
    return 0;
}
