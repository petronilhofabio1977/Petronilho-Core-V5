#include <iostream>
#include "core/sys/sys_core.hpp"
#include "core/sys/handle.hpp"
#include "geometric_wing/spatial_node.hpp"
#include "relational/hash_table.hpp"

using namespace super_core;

int main() {
    sys::boot_info();
    const size_t RAM_SIZE = 16 * 1024 * 1024;
    void* buffer = nullptr;
    posix_memalign(&buffer, 64, RAM_SIZE);
    sys::G_POOL_BASE = (uint8_t*)buffer;
    sys::Arena arena(buffer, RAM_SIZE);

    // Criar Tabela Hash (ID -> Ponto Geométrico)
    relational::HashTable<uint32_t, geometric::Point2D> registry(arena, 1000);

    // 1. Criar um ponto "alvo"
    auto h_ponto = arena.allocate<geometric::Point2D>();
    h_ponto->x = 42.0f; h_ponto->y = 7.0f;

    // 2. Registrar na Tabela Relacional com ID 123
    registry.insert(123, h_ponto);

    // 3. Buscar instantaneamente
    auto achado = registry.get(123);
    
    if(!achado.is_null()) {
        std::cout << "[RELATIONAL] Ponto ID 123 encontrado! Coordenadas: (" 
                  << achado->x << ", " << achado->y << ")" << std::endl;
    }

    free(buffer);
    return 0;
}
