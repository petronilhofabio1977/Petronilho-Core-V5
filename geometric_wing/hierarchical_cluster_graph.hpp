/**
 * @file hierarchical_cluster_graph.hpp
 * @brief Grafo de Clusters Recursivo (Recursive Cluster Topology).
 * * ALGORITMO: Decomposição Hierárquica de Grafos.
 * REFERÊNCIA: CLRS Capítulo 22 (Algoritmos de Grafos - Representações).
 * * POR QUE: Permite roteamento em tempo constante O(1) dentro de um cluster 
 * e O(log N) entre clusters, espelhando a hierarquia física de supercomputadores.
 */

#pragma once
#include "core/sys/handle.hpp"

namespace super_core::geometric {

struct alignas(64) ClusterNode {
    uint32_t cluster_id;
    sys::Handle<ClusterNode> parent_cluster;
    
    // Handle para a topologia interna (definida em relational/full_mesh_topology.hpp)
    // Usamos uint32_t para manter o desacoplamento de tipos (Type Erasing)
    uint32_t internal_topology_handle; 

    // O(1) - Navegação entre níveis da hierarquia
};

} // namespace super_core::geometric