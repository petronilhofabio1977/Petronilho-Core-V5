// Layer: L1 | Version: 1.1.0 | Author: Fabio Petronilho de Oliveira
//
// ================================================================
// collision.hpp - Deteccao de Colisao entre Regioes
// ================================================================
//
// O QUE ESSE CODIGO FAZ:
// Verifica se duas regioes espaciais se sobrepoem.
// No contexto do Core: detecta sobreposicao de intervalos
// de tempo, ranges de IDs ou janelas de dados.
//
// ALGORITMO: Separating Axis Theorem simplificado
// BASE TEORICA: Cormen Cap.33 - Computational Geometry
// Cormen Cap.33.1: dois segmentos se intersectam se e
// somente se nao existe um eixo separador entre eles.
// Para AABBs (Axis-Aligned Bounding Boxes) a verificacao
// e O(1): testa separacao em cada eixo independentemente.
//
// APLICACAO NO CORE:
// Detectar sobreposicao de janelas de tempo em metricas,
// colisao de ranges de Security IDs, ou overlap de
// intervalos de timestamps em range queries da B-Tree.
//
// Complexidade: O(1) por par de regioes
// ================================================================

#pragma once
#include "geometric_wing/spatial_node.hpp"

namespace petronilho::geometric {

    // ============================================================
    // check_collision_2d - Colisao em 2 dimensoes
    // BASE TEORICA: Cormen Cap.33.1 - Segment Intersection
    // Dois AABBs colidem se NAO ha separacao em nenhum eixo.
    // Complexidade: O(1)
    // ============================================================
    [[nodiscard]]
    inline bool check_collision_2d(
        const BoundingBox& a,
        const BoundingBox& b) noexcept
    {
        // Cormen Cap.33.1: teste de separacao por eixo
        // Se separados em qualquer eixo, nao colidem
        return (a.min_p.x <= b.max_p.x && a.max_p.x >= b.min_p.x) &&
               (a.min_p.y <= b.max_p.y && a.max_p.y >= b.min_p.y);
    }

    // ============================================================
    // check_collision_time - Colisao em intervalo de tempo
    // Caso especial 1D para range queries de timestamps
    // Aplicacao: verificar se dois intervalos de metricas
    // se sobrepoem na B-Tree de time-series
    // Complexidade: O(1)
    // ============================================================
    [[nodiscard]]
    inline bool check_collision_time(
        uint64_t a_start, uint64_t a_end,
        uint64_t b_start, uint64_t b_end) noexcept
    {
        return a_start <= b_end && a_end >= b_start;
    }

} // namespace petronilho::geometric