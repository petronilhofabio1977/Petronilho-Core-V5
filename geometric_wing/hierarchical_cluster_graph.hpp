// Layer: L1 | Version: 1.1.0 | Author: Fabio Petronilho de Oliveira
//
// ================================================================
// hierarchical_cluster_graph.hpp - Grafo de Clusters Hierarquico
// ================================================================
//
// O QUE ESSE CODIGO FAZ:
// Representa a topologia de rede como hierarquia de clusters.
// No contexto de segurança: mapeia a estrutura fisica da rede
// para detectar anomalias por segmento. Um ataque DDoS aparece
// como volume anômalo em um cluster especifico antes de se
// propagar para os outros.
//
// ALGORITMO: Decomposicao Hierarquica de Grafos
// BASE TEORICA: Cormen Cap.22 - Elementary Graph Algorithms
// Cormen Cap.22.1: representacao de grafos por lista de adjacencia
// Cada ClusterNode e um vertice. parent_cluster e a aresta
// para o cluster de nivel superior.
// Complexidade navegacao: O(1) dentro do cluster
//                         O(log N) entre clusters
//
// APLICACAO EM SEGURANÇA:
// Detectar de qual segmento de rede vem o trafego anomalo.
// Se um cluster tem volume 10x acima do P99 normal enquanto
// os outros estao normais, o ataque esta localizado ali.
// Isso e impossivel detectar sem hierarquia de clusters.
//
// Complexidade Tempo : O(1) navegacao local, O(log N) global
// Complexidade Espaco: O(N) onde N = numero de clusters
// ================================================================

#pragma once
#include <cstdint>

namespace petronilho::geometric {

    // ============================================================
    // ClusterNode - Vertice do grafo hierarquico de rede
    //
    // CALCULO DE TAMANHO para 64 bytes:
    // uint32_t cluster_id              =  4 bytes
    // uint32_t parent_cluster_index    =  4 bytes
    // uint32_t internal_topology_handle=  4 bytes
    // uint32_t depth                   =  4 bytes
    // uint64_t packet_count            =  8 bytes
    // uint64_t anomaly_threshold       =  8 bytes
    // uint64_t last_seen_ns            =  8 bytes
    // uint8_t  cluster_type            =  1 byte
    // uint8_t  flags                   =  1 byte
    // uint8_t  _pad[18]                = 18 bytes
    //                                  = 64 bytes TOTAL
    //
    // Cormen Cap.22: cada no carrega informacoes do vertice
    // necessarias para os algoritmos de grafos (BFS/DFS)
    // ============================================================
    struct alignas(64) ClusterNode {
        uint32_t m_cluster_id;           // ID unico do cluster
        uint32_t m_parent_index;         // indice do cluster pai no pool
        uint32_t m_topology_handle;      // handle para full_mesh_topology
        uint32_t m_depth;                // profundidade na hierarquia

        uint64_t m_packet_count;         // pacotes recebidos (deteccao DDoS)
        uint64_t m_anomaly_threshold;    // limite para alerta de anomalia
        uint64_t m_last_seen_ns;         // timestamp ultimo pacote

        uint8_t  m_cluster_type;         // 0=edge, 1=core, 2=datacenter
        uint8_t  m_flags;                // bits de estado
        uint8_t  m_pad[18];              // padding para 64 bytes exatos
    };

    static_assert(sizeof(ClusterNode) == 64,
        "ClusterNode deve ter exatamente 64 bytes");
    static_assert(alignof(ClusterNode) == 64,
        "ClusterNode deve estar alinhado a 64 bytes");

    // Indice sentinela para cluster raiz sem pai
    static constexpr uint32_t CLUSTER_ROOT = 0xFFFFFFFFu;

} // namespace petronilho::geometric
```

**Commit:**

Título: `Refatora ClusterNode para 64 bytes e contexto seguranca v1.1`

Descrição:
```
- Corrige tamanho para 64 bytes exatos com static_assert
- Adiciona packet_count e anomaly_threshold para deteccao DDoS
- Adiciona last_seen_ns para correlacao temporal de ataques
- Remove Handle<T> de 12 bytes impossibilitava 64 bytes
- Documenta aplicacao em seguranca de rede
- Corrige namespace para petronilho::geometric
