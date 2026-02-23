// Layer: L1 | Version: 1.1.0 | Author: Fabio Petronilho de Oliveira
//
// ================================================================
// graph_packed.hpp - Grafo Estatico Compacto (Forward Star)
// ================================================================
//
// O QUE ESSE CODIGO FAZ:
// Representa grafo como dois arrays contiguos: nos e arestas.
// Arestas de um mesmo no ficam side-by-side na memoria,
// permitindo que o prefetcher carregue todas antes de precisar.
//
// ALGORITMO: Static Forward Star
// BASE TEORICA: Cormen Cap.22.1 - Representacao de Grafos
// Cormen Cap.22.1 compara lista de adjacencia vs matriz.
// Forward Star e variante da lista de adjacencia onde todas
// as arestas ficam em um unico array ordenado por no de origem.
// Resultado: acesso as arestas de um no e O(1) por indice,
// sem pointer chasing entre nos da lista encadeada.
//
// APLICACAO EM SEGURANÇA:
// Modelar a topologia da rede monitorada como grafo.
// Nos = dispositivos, Arestas = conexoes com capacidade.
// Fluxo maximo (flow_push_relabel.cpp) calcula a capacidade
// maxima de um ataque DDoS atraves da rede.
//
// Complexidade Espaco: O(V + E) onde V=nos, E=arestas
// Acesso a aresta: O(1) via offset + indice
// ================================================================

#pragma once
#include <cstdint>

namespace petronilho::relational {

    // ============================================================
    // Edge - Aresta do grafo com capacidade de fluxo
    // Cormen Cap.26: aresta (u,v) com capacidade c(u,v)
    // rev: indice da aresta reversa no grafo residual
    // ============================================================
    struct Edge {
        uint32_t m_to;        // no destino
        uint32_t m_rev;       // indice da aresta reversa
        float    m_capacity;  // capacidade maxima
        float    m_flow;      // fluxo atual
    };

    // ============================================================
    // Node - No com offset para suas arestas no array
    // Cormen Cap.22.1: lista de adjacencia como array
    // m_first_edge: inicio das arestas deste no no array
    // m_edge_count: quantas arestas saem deste no
    // ============================================================
    struct Node {
        uint32_t m_first_edge; // offset no array de arestas
        uint32_t m_edge_count; // grau de saida do no
        float    m_excess;     // excesso de fluxo (Push-Relabel)
        uint32_t m_height;     // altura do no (Push-Relabel)
    };

    // ============================================================
    // GraphPacked - Container do grafo completo
    // Dois arrays contiguos: nos e arestas na Arena
    // ============================================================
    struct GraphPacked {
        Node*    m_nodes;      // array de nos na Arena
        Edge*    m_edges;      // array de arestas na Arena
        uint32_t m_node_count;
        uint32_t m_edge_count;

        // Acesso O(1) as arestas de um no
        [[nodiscard]]
        Edge* edges_of(uint32_t node_idx) noexcept {
            return m_edges + m_nodes[node_idx].m_first_edge;
        }

        [[nodiscard]]
        uint32_t degree(uint32_t node_idx) const noexcept {
            return m_nodes[node_idx].m_edge_count;
        }
    };

} // namespace petronilho::relational
