/**
 * @file full_mesh_topology.hpp
 * @brief Topologia de Conectividade Total (Clique K13).
 * * ALGORITMO: Complete Graph Topology.
 * REFERÊNCIA: CLRS Apêndice B.4 (Grafos e Árvores).
 * * POR QUE: Utilizado para roteamento de baixíssima latência dentro de clusters.
 * Permite broadcast de dados em O(1) ciclos.
 */

#pragma once
#include <cstdint>

namespace super_core::relational {

struct alignas(64) FullMeshTopology {
    static constexpr uint32_t NODES = 13;
    // Matriz de adjacência bit-packed (13 bits por linha)
    uint16_t adj[NODES]; 
    
    // Pesos das conexões otimizados para AVX-512
    alignas(64) float weights[NODES * NODES];
};

} // namespace super_core::relational