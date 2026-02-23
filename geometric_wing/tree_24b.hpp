// Layer: L1 | Version: 1.1.0 | Author: Fabio Petronilho de Oliveira
//
// ================================================================
// tree_24b.hpp - Red-Black Tree Compacta para Cache L1
// ================================================================
//
// O QUE ESSE CODIGO FAZ:
// No de Red-Black Tree dimensionado para 24 bytes, permitindo
// que 2 nos caibam em uma unica cache line de 64 bytes com
// 16 bytes de sobra para o prefetcher trabalhar.
//
// ALGORITMO: Red-Black Tree
// BASE TEORICA: Cormen Cap.13 - Red-Black Trees
// Cormen Teorema 13.1: RB-Tree com n nos internos tem
// altura maxima 2*log(n+1), garantindo O(log n) para
// busca, insercao e remocao.
//
// POR QUE 24 BYTES E NAO 64 BYTES (como BTreeNode64)?
// BTreeNode64: otimizado para disco/RAM grande, muitas chaves
// Node24: otimizado para L1 cache, nos pequenos e frequentes
// 2 Node24 por cache line = menos misses em travessia da arvore
//
// DIFERENCA vs BTreeNode64:
// BTreeNode64: muitas chaves por no, menos niveis, bom para disco
// Node24: poucos dados por no, mais niveis, otimo para L1 cache
//
// Complexidade: O(log n) busca, insercao, remocao
// ================================================================

#pragma once
#include <cstdint>

namespace petronilho::geometric {

    enum class Color : uint8_t { RED = 0, BLACK = 1 };

    // ============================================================
    // Node24 - No de RB-Tree em 24 bytes exatos
    //
    // CALCULO DE TAMANHO:
    // uint32_t left    = 4 bytes (indice no pool)
    // uint32_t right   = 4 bytes (indice no pool)
    // uint32_t parent  = 4 bytes (indice no pool)
    // uint32_t key     = 4 bytes (chave uint32_t)
    // Color    color   = 1 byte
    // uint8_t  _pad[3] = 3 bytes (padding explicito)
    //                  = 24 bytes TOTAL
    //
    // NOTA: filhos como indices uint32_t em vez de Handle<T>
    // Handle<T> refatorado tem 12 bytes (uint32_t + uint8_t*)
    // impossivel caber 3 handles + chave + cor em 24 bytes.
    // Indice uint32_t e suficiente: pool base e passado
    // separadamente na operacao de busca, igual ao BTreeNode64.
    //
    // Cormen Cap.13: sentinel NIL representado por indice 0
    // ============================================================
    struct alignas(8) Node24 {
        uint32_t m_left;    // indice do filho esquerdo no pool
        uint32_t m_right;   // indice do filho direito no pool
        uint32_t m_parent;  // indice do pai no pool
        uint32_t m_key;     // chave de busca
        Color    m_color;   // RED ou BLACK para balanceamento
        uint8_t  m_pad[3];  // padding explicito para 24 bytes
    };

    // Verificacao obrigatoria pela governanca
    static_assert(sizeof(Node24) == 24,
        "Node24 deve ter exatamente 24 bytes");
    static_assert(alignof(Node24) == 8,
        "Node24 deve estar alinhado a 8 bytes");

    // Dois Node24 por cache line verificado em compile-time
    static_assert(64 / sizeof(Node24) >= 2,
        "Node24 deve permitir ao menos 2 nos por cache line");

    // Sentinel: indice 0 representa NIL (Cormen Cap.13)
    static constexpr uint32_t RB_NIL = 0u;

} // namespace petronilho::geometric