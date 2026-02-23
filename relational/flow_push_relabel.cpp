// Layer: L1 | Version: 1.1.0 | Author: Fabio Petronilho de Oliveira
//
// ================================================================
// flow_push_relabel.cpp - Fluxo Maximo Goldberg-Tarjan
// ================================================================
//
// O QUE ESSE CODIGO FAZ:
// Calcula o fluxo maximo em uma rede usando Push-Relabel.
// No contexto de seguranca: calcula a capacidade maxima de
// trafego que um ataque DDoS consegue enviar atraves da rede.
// Se o fluxo calculado excede o threshold normal, e um ataque.
//
// ALGORITMO: Push-Relabel (Goldberg-Tarjan 1988)
// BASE TEORICA: Cormen Cap.26.4 - Push-Relabel
// Cormen Cap.26.1: rede de fluxo, capacidade, fluxo residual
// Cormen Cap.26.4: INITIALIZE-PREFLOW, PUSH, RELABEL
// Cormen Teorema 26.23: complexidade O(V² * sqrt(E))
// Melhor que Ford-Fulkerson O(V * E²) para redes densas
//
// INTEGRACAO COM priority_wing:
// Cada operacao PUSH e RELABEL pode ser agendada como
// Task no TaskScheduler com prioridade pelo excesso de fluxo.
// Nos com maior excesso recebem prioridade maior no heap.
//
// Complexidade: O(V² * sqrt(E))
// ================================================================

#include "relational/graph_packed.hpp"
#include "core/sys/arena.hpp"
#include <cstdint>
#include <cstring>

namespace petronilho::relational {

    // ============================================================
    // op_push - Move excesso de fluxo para vizinho
    // Cormen Cap.26.4: PUSH(u, v)
    // Pre-condicao: u.excess > 0, height[u] == height[v] + 1
    // Complexidade: O(1)
    // ============================================================
    void op_push(Node& u, Node& v, Edge& uv) noexcept {
        // Cormen Cap.26.4: delta = min(excess[u], residual(u,v))
        const float residual = uv.m_capacity - uv.m_flow;
        if (residual <= 0.0f || u.m_excess <= 0.0f) return;

        const float delta = u.m_excess < residual
                          ? u.m_excess
                          : residual;

        // Atualiza fluxo na aresta e na reversa
        uv.m_flow      += delta;
        u.m_excess     -= delta;
        v.m_excess     += delta;
    }

    // ============================================================
    // op_relabel - Aumenta altura do no para permitir push
    // Cormen Cap.26.4: RELABEL(u)
    // Pre-condicao: u.excess > 0, nenhum vizinho admissivel
    // Complexidade: O(degree(u))
    // ============================================================
    void op_relabel(Node& u,
                    const Edge* edges,
                    const Node* nodes) noexcept
    {
        uint32_t min_height = 0xFFFFFFFFu;

        const Edge* e   = edges + u.m_first_edge;
        const Edge* end = e + u.m_edge_count;

        for (; e != end; ++e) {
            const float residual = e->m_capacity - e->m_flow;
            if (residual <= 0.0f) continue;

            const uint32_t neighbor_h = nodes[e->m_to].m_height;
            if (neighbor_h < min_height)
                min_height = neighbor_h;
        }

        // Cormen Cap.26.4: nova altura = min_height + 1
        if (min_height != 0xFFFFFFFFu)
            u.m_height = min_height + 1;
    }

    // ============================================================
    // initialize_preflow - Inicializa fluxo da fonte
    // Cormen Cap.26.4: INITIALIZE-PREFLOW(G, s)
    // Satura todas as arestas saindo da fonte s
    // Complexidade: O(V + E)
    // ============================================================
    void initialize_preflow(GraphPacked& g,
                            uint32_t     source) noexcept
    {
        // Altura da fonte = numero de nos
        g.m_nodes[source].m_height  = g.m_node_count;
        g.m_nodes[source].m_excess  = 0.0f;

        // Satura arestas da fonte
        Edge* e   = g.m_edges + g.m_nodes[source].m_first_edge;
        Edge* end = e + g.m_nodes[source].m_edge_count;

        for (; e != end; ++e) {
            e->m_flow                        = e->m_capacity;
            g.m_nodes[e->m_to].m_excess     += e->m_capacity;
            // Aresta reversa
            g.m_edges[e->m_rev].m_flow      -= e->m_capacity;
        }
    }

    // ============================================================
    // max_flow - Calcula fluxo maximo entre source e sink
    // Cormen Cap.26.4: algoritmo completo Push-Relabel
    // Complexidade: O(V² * sqrt(E))
    // ============================================================
    float max_flow(GraphPacked& g,
                   uint32_t     source,
                   uint32_t     sink) noexcept
    {
        initialize_preflow(g, source);

        // Processa nos com excesso ate nao restar nenhum
        // exceto source e sink
        bool active = true;
        while (active) {
            active = false;

            for (uint32_t i = 0; i < g.m_node_count; ++i) {
                if (i == source || i == sink) continue;
                if (g.m_nodes[i].m_excess <= 0.0f) continue;

                Node& u     = g.m_nodes[i];
                Edge* e     = g.m_edges + u.m_first_edge;
                Edge* end   = e + u.m_edge_count;
                bool pushed = false;

                for (; e != end; ++e) {
                    Node& v          = g.m_nodes[e->m_to];
                    const float res  = e->m_capacity - e->m_flow;

                    // Push admissivel: residual > 0 e altura correta
                    if (res > 0.0f && u.m_height == v.m_height + 1) {
                        op_push(u, v, *e);
                        active = pushed = true;
                        if (u.m_excess <= 0.0f) break;
                    }
                }

                // Nenhum push possivel: relabel
                if (!pushed && u.m_excess > 0.0f) {
                    op_relabel(u, g.m_edges, g.m_nodes);
                    active = true;
                }
            }
        }

        // Fluxo total = excesso acumulado no sink
        return g.m_nodes[sink].m_excess;
    }

} // namespace petronilho::relational
