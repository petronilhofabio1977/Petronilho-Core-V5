/**
 * @file disjoint_set.hpp
 * @brief Union-Find com Path Compression e Union by Rank.
 * * ALGORITMO: Disjoint-Set Forest (CLRS Capítulo 21).
 * REFERÊNCIA: CLRS Capítulo 21 (Estruturas de Dados para Conjuntos Disjuntos).
 * * POR QUE: Proporciona tempo de execução quase linear para detectar ciclos
 * em grafos massivos.
 */

#pragma once
#include "core/sys/handle.hpp"

namespace super_core::relational {

struct DSNode {
    uint32_t parent;
    uint32_t rank;
};

class DisjointSet {
    sys::Handle<DSNode> nodes;
public:
    // Find com Path Compression: O(alpha(N))
    uint32_t find_set(uint32_t i, DSNode* arr) noexcept {
        if (i != arr[i].parent)
            arr[i].parent = find_set(arr[i].parent, arr);
        return arr[i].parent;
    }
};

} // namespace super_core::relational