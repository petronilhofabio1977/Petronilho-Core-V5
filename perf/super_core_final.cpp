#include <iostream>
#include "core/sys/sys_core.hpp"
#include "core/sys/handle.hpp"
#include "core/sys/memory.hpp"
#include "core/sys/arena_atomic.hpp" // Importante: ScalableArena está aqui
#include "geometric_wing/spatial_node.hpp"
#include "geometric_wing/collision.hpp"
#include "relational/hash_table.hpp"

using namespace super_core;

int main() {
    sys::boot_info();
    const size_t RAM_SIZE = 16 * 1024 * 1024;
    
    void* buffer = sys::aligned_alloc_memory(64, RAM_SIZE);
    if (!buffer) return 1;

    sys::G_POOL_BASE = (uint8_t*)buffer;
    
    // Usando a ScalableArena definida anteriormente
    sys::ScalableArena arena(buffer, RAM_SIZE);

    relational::HashTable<uint32_t, geometric::BoundingBox> scene(arena, 100);

    auto h_a = arena.allocate<geometric::BoundingBox>();
    h_a->min_p = {0, 0}; h_a->max_p = {10, 10};
    scene.insert(1, h_a);

    auto h_b = arena.allocate<geometric::BoundingBox>();
    h_b->min_p = {5, 5}; h_b->max_p = {15, 15};
    scene.insert(2, h_b);

    auto obj1 = scene.get(1);
    auto obj2 = scene.get(2);

    if (!obj1.is_null() && !obj2.is_null() && geometric::check_collision(*obj1, *obj2)) {
        std::cout << "[SUPER-CORE] SUCESSO CROSS-PLATFORM: Colisao detectada!" << std::endl;
    }

    sys::aligned_free_memory(buffer);
    return 0;
}
