/**
 * @file graph_packed.hpp
 * @brief Representação de Grafo Estático e Compacto.
 * * ALGORITMO: Adjacency List / Static Forward Star (CLRS Capítulo 22.1).
 * REFERÊNCIA: CLRS Capítulo 22 (Algoritmos de Grafos).
 * * POR QUE: Minimiza o 'pointer chasing'. As arestas de um nó são contíguas,
 * permitindo que o Hardware Prefetcher carregue os dados antes do algoritmo solicitar.
 */

#pragma once
#include "core/sys/handle.hpp"

namespace super_core::relational {

struct Edge {
    uint32_t to;         // Destino (índice)
    float capacity;      // Fluxo Máximo
    float flow;          // Fluxo Atual
    uint32_t rev;        // Índice da aresta reversa (Residual Graph)
};

struct Node {
    uint32_t first_edge; // Offset no array de arestas
    uint32_t edge_count; // Grau do nó
};

struct GraphPacked {
    sys::Handle<Node> nodes;
    sys::Handle<Edge> edges;
    uint32_t node_count;
    uint32_t edge_count;
};

} // namespace super_core::relational