// Layer: L1 | Version: 1.1.0 | Author: Fabio Petronilho de Oliveira
//
// ================================================================
// full_mesh_topology.hpp - Topologia Full Mesh (Clique K13)
// ================================================================
//
// O QUE ESSE CODIGO FAZ:
// Representa uma topologia onde cada no esta conectado a todos
// os outros. Usado dentro de um ClusterNode para modelar a
// conectividade interna de um segmento de rede.
//
// ALGORITMO: Complete Graph (Clique)
// BASE TEORICA: Cormen Apendice B.4 - Grafos e Arvores
// Cormen B.4: grafo completo K_n tem n*(n-1)/2 arestas.
// K13 tem 13*12/2 = 78 arestas.
// Representacao por matriz de adjacencia bit-packed:
// adj[i] e um bitmask de 13 bits onde bit j=1 significa
// que existe aresta de i para j.
//
// APLICACAO EM SEGURANÇA:
// Dentro de um cluster de 13 dispositivos monitorados,
// qualquer dispositivo pode comunicar com qualquer outro
// diretamente. Broadcast em O(1): setar todos os bits.
// Detectar isolamento: um no com adj[i]=0 foi cortado,
// possivel indicador de ataque ou falha.
//
// CALCULO DE TAMANHO:
// uint16_t adj[13]          = 26 bytes
// uint8_t  _pad[38]         = 38 bytes
//                           = 64 bytes (primeira cache line)
// float weights[13*13=169]  = 676 bytes (arestas com pesos)
// Total alinhado            = 64 + 704 = 768 bytes
//
// Complexidade broadcast : O(1) via bitmask
// Complexidade vizinhos  : O(1) via adj[i]
// ================================================================

#pragma once
#include <cstdint>

namespace petronilho::relational {

    struct alignas(64) FullMeshTopology {
        static constexpr uint32_t NODES = 13;

        // Matriz de adjacencia bit-packed
        // adj[i]: bitmask dos vizinhos do no i
        // bit j=1 significa aresta i->j existe
        uint16_t m_adj[NODES];
        uint8_t  m_pad[38]; // padding para fechar 64 bytes

        // Pesos das conexoes alinhados para AVX512
        // weights[i * NODES + j] = peso da aresta i->j
        alignas(64) float m_weights[NODES * NODES];

        // ============================================================
        // connect - Adiciona aresta bidirecional i <-> j
        // Cormen B.4: grafo nao-direcionado
        // Complexidade: O(1)
        // ============================================================
        void connect(uint32_t i, uint32_t j) noexcept {
            if (i >= NODES || j >= NODES) return;
            m_adj[i] |= static_cast<uint16_t>(1u << j);
            m_adj[j] |= static_cast<uint16_t>(1u << i);
        }

        // ============================================================
        // broadcast - Conecta no i a todos os outros
        // Cormen B.4: clique, O(1) via bitmask
        // ============================================================
        void broadcast(uint32_t i) noexcept {
            if (i >= NODES) return;
            // Todos os 13 bits ativos exceto o proprio no
            m_adj[i] = static_cast<uint16_t>(
                ((1u << NODES) - 1u) & ~(1u << i));
        }

        // Verifica se no i esta isolado (possivel ataque)
        [[nodiscard]]
        bool is_isolated(uint32_t i) const noexcept {
            if (i >= NODES) return true;
            return m_adj[i] == 0;
        }

        // Verifica conectividade entre i e j
        [[nodiscard]]
        bool connected(uint32_t i, uint32_t j) const noexcept {
            if (i >= NODES || j >= NODES) return false;
            return (m_adj[i] & (1u << j)) != 0;
        }
    };

    static_assert(offsetof(FullMeshTopology, m_weights) == 64,
        "m_weights deve comecar na segunda cache line");

} // namespace petronilho::relational
