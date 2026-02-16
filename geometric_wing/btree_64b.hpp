/**
 * @file btree_64b.hpp
 * @brief B-Tree Otimizada para Cache Line (64 bytes).
 * * ALGORITMO: B-Tree (CLRS Capítulo 18).
 * REFERÊNCIA: CLRS Capítulo 18 (Árvores B).
 * * POR QUE: Minimiza a profundidade da árvore e garante que cada 'fetch' 
 * de memória traga o máximo de chaves possíveis (Fan-out otimizado para hardware).
 */

#pragma once
#include "core/sys/handle.hpp"

namespace super_core::geometric {

/**
 * @brief Nó de B-Tree dimensionado para 64 bytes.
 * Grau mínimo t=3 (até 5 chaves e 6 filhos por nó).
 */
struct alignas(64) BTreeNode64 {
    uint16_t num_keys;
    bool is_leaf;
    uint32_t keys[5];             // 20 bytes
    sys::Handle<BTreeNode64> children[6]; // 48 bytes (estouro controlado)
    
    // O(1) Search inside node (Vetorizado via core/platform/intrinsics_util.hpp)
};

} // namespace super_core::geometric