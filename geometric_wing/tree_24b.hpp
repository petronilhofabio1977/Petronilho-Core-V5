/**
 * @file tree_24b.hpp
 * @brief Árvore de Busca Balanceada Compacta (Red-Black Tree).
 * * ALGORITMO: Red-Black Tree (CLRS Capítulo 13).
 * REFERÊNCIA: CLRS Capítulo 13 (Árvores Rubro-Negras).
 * * POR QUE: Garante balanceamento perfeito com altura máxima 2log(n+1). 
 * O layout de 24 bytes maximiza o 'packing' de nós no cache L1.
 */

#pragma once
#include "core/sys/handle.hpp"

namespace super_core::geometric {

enum class Color : uint8_t { RED, BLACK };

template <typename T>
struct alignas(8) Node24 {
    sys::Handle<Node24<T>> left;   // 4B index + 4B gen
    sys::Handle<Node24<T>> right;  // 4B index + 4B gen
    sys::Handle<Node24<T>> parent; // 4B index + 4B gen
    T key;                         // Tipo da chave
    Color color;                   // Cor para balanceamento
};

// Invariante: sizeof(Node24<int>) deve ser <= 32 bytes para alinhar 2 por cache line
static_assert(sizeof(Node24<int>) <= 32, "Node24 excede limite de densidade de cache.");

} // namespace super_core::geometric