// Layer: L1 | Version: 1.1.0 | Author: Fabio Petronilho de Oliveira
//
// ================================================================
// btree_64b.hpp - B-Tree Otimizada para Cache Line de 64 bytes
// ================================================================
//
// O QUE ESSE CODIGO FAZ:
// Implementa no de B-Tree dimensionado exatamente para 64 bytes,
// o tamanho de uma cache line x86. Usado para indexacao de
// sessoes FIX/FAST, range queries de metricas e GPS da Arena.
//
// ALGORITMO: B-Tree
// BASE TEORICA: Cormen Cap.18 - B-Trees
// Cormen Teorema 18.1: altura de B-Tree com n chaves e grau t
// e h <= log_t((n+1)/2), ou seja O(log_t n).
// Com t=2 (grau minimo ajustado): busca em O(log_2 n)
//
// POR QUE B-TREE E NAO BST (Cormen Cap.12)?
// BST: cada nivel = um no = um acesso a memoria = cache miss
// B-Tree: cada nivel = um no com MULTIPLAS chaves = uma cache
// line = todas as chaves do no chegam juntas no L1 cache.
// Com 64 bytes por no o processador traz 3 chaves + filhos
// em UM acesso a memoria. Isso e Hardware Determinism.
//
// TRES USOS NO PETRONILHO CORE V5:
// 1. Indexacao de sessoes FIX/FAST por Security ID
// 2. Range queries de metricas por timestamp (P99 historico)
// 3. GPS da Arena: encontrar pacote especifico sem ler 2GB
//
// Complexidade Tempo : O(log_t n) busca, insercao, remocao
// Complexidade Espaco: Theta(n)
// ================================================================

#pragma once
#include "core/sys/handle.hpp"
#include <cstdint>
#include <cstddef>

namespace petronilho::geometric {

    // ============================================================
    // BTreeNode64 - No de B-Tree com tamanho exato de 64 bytes
    //
    // CALCULO DE TAMANHO (deve fechar em 64 bytes exatos):
    //
    // uint16_t num_keys      =  2 bytes
    // bool     is_leaf       =  1 byte
    // uint8_t  _pad          =  1 byte  (padding explicito)
    // uint32_t keys[3]       = 12 bytes (3 chaves, grau t=2)
    // uint32_t children[4]   = 16 bytes (4 indices de filhos)
    // uint8_t  _reserved[32] = 32 bytes (espaco para expansao)
    //                        = 64 bytes TOTAL
    //
    // NOTA SOBRE DESIGN:
    // Handle<T> original (uint32_t + uint8_t*) = 12 bytes cada.
    // 4 handles = 48 bytes so para filhos, impossivel caber.
    // Solucao: filhos guardados como indices uint32_t (4 bytes)
    // relativos ao pool da Arena, igual ao principio do Handle
    // mas sem o ponteiro base (que e compartilhado pelo pool).
    // O pool base e passado separadamente na operacao de busca.
    //
    // Cormen Cap.18 Sec.18.1: grau minimo t=2 significa
    // cada no tem no minimo 1 chave e no maximo 3 chaves,
    // e no maximo 4 filhos. Isso cabe em 64 bytes.
    // ============================================================
    struct alignas(64) BTreeNode64 {
        uint16_t m_num_keys;       // numero de chaves ativas
        bool     m_is_leaf;        // verdadeiro se no folha
        uint8_t  m_pad;            // padding explicito

        uint32_t m_keys[3];        // chaves ordenadas
        uint32_t m_children[4];    // indices dos filhos no pool

        uint8_t  m_reserved[32];   // reservado para expansao
                                   // ex: valores, timestamps,
                                   // flags de auditoria
    };

    // Verificacao em compile-time obrigatoria pela governanca
    static_assert(sizeof(BTreeNode64) == 64,
        "BTreeNode64 deve ter exatamente 64 bytes");
    static_assert(alignof(BTreeNode64) == 64,
        "BTreeNode64 deve estar alinhado a 64 bytes");

} // namespace petronilho::geometric