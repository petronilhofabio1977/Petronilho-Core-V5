// Layer: L1 | Version: 1.1.0 | Author: Fabio Petronilho de Oliveira
//
// ================================================================
// spatial_node.hpp - Primitivas Geometricas 2D
// ================================================================
//
// O QUE ESSE CODIGO FAZ:
// Define as primitivas geometricas usadas pela geometric_wing:
// Point2D para coordenadas e BoundingBox para regioes.
// No contexto do Core: coordenadas representam (timestamp, id)
// ou (latencia, volume) em espacos de metricas 2D.
//
// ALGORITMO: Ordem Lexicografica em R²
// BASE TEORICA: Cormen Cap.33 - Computational Geometry
// Cormen Cap.33 usa ordem lexicografica para pontos:
// p < q se p.x < q.x, ou p.x == q.x e p.y < q.y.
// Isso garante ordem total unica e consistente com a
// B-Tree do btree_64b.hpp.
//
// CORRECAO vs versao anterior:
// operator< usava distancia a origem (x²+y²) que e incorreto.
// Dois pontos distintos podem ter mesma distancia a origem
// causando colisoes silenciosas na B-Tree.
// Substituido por ordem lexicografica O(1) sem multiplicacao.
// ================================================================

#pragma once
#include <cstdint>

namespace petronilho::geometric {

    // ============================================================
    // Point2D - Ponto em espaco 2D
    // Cormen Cap.33: ponto como par ordenado (x, y)
    // No Core: (timestamp_ns, security_id) ou (latencia, volume)
    // ============================================================
    struct Point2D {
        float m_x;
        float m_y;

        // CORRECAO: ordem lexicografica em vez de distancia
        // Cormen Cap.33: p < q sse p.x < q.x ou
        // (p.x == q.x e p.y < q.y)
        // Garante ordem total unica para uso na B-Tree
        [[nodiscard]]
        bool operator<(const Point2D& other) const noexcept {
            if (m_x != other.m_x) return m_x < other.m_x;
            return m_y < other.m_y;
        }

        [[nodiscard]]
        bool operator==(const Point2D& other) const noexcept {
            return m_x == other.m_x && m_y == other.m_y;
        }
    };

    // ============================================================
    // BoundingBox - Regiao retangular alinhada aos eixos (AABB)
    // Cormen Cap.33.1: segmento definido por dois pontos extremos
    // min_p = canto inferior esquerdo
    // max_p = canto superior direito
    // Invariante: min_p <= max_p em ambos os eixos
    // ============================================================
    struct BoundingBox {
        Point2D m_min;
        Point2D m_max;

        // Verifica invariante: min <= max em ambos eixos
        [[nodiscard]]
        bool is_valid() const noexcept {
            return m_min.m_x <= m_max.m_x &&
                   m_min.m_y <= m_max.m_y;
        }
    };

} // namespace petronilho::geometric