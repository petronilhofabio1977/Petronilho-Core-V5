// Layer: L1 | Version: 1.1.0 | Author: Fabio Petronilho de Oliveira
//
// ================================================================
// vcore_avx512.cpp - Kernel Vetorial AVX512 512 bits
// ================================================================
//
// O QUE ESSE CODIGO FAZ:
// Processa blocos de floats usando registradores ZMM de 512 bits.
// Cada instrucao opera em 16 floats simultaneamente.
// Inclui busca branchless de 16 chaves simultaneas via masking.
//
// ALGORITMO 1: Vetorizacao com FMA 512 bits
// BASE TEORICA: Cormen Cap.27 - Multithreaded Algorithms
// SIMD e paralelismo de dados: ao inves de paralelismo
// de threads (Cap.27), explora paralelismo dentro de uma
// unica instrucao. O principio e o mesmo: dividir o trabalho
// em partes independentes executadas simultaneamente.
// Complexidade: O(n/16) onde n = count
//
// ALGORITMO 2: Busca Branchless com Masking
// BASE TEORICA: Cormen Cap.11 - Hash Tables
// find_match_512 implementa busca por igualdade em O(1)
// para grupos de 16 chaves. Equivale a um bucket de hash
// com resolucao por SIMD em vez de lista encadeada.
// Complexidade: O(1) para 16 chaves, O(n/16) para n chaves
// ================================================================

#include "core/platform/intrinsics_util.hpp"
#include <immintrin.h>
#include <cstdint>
#include <cstddef>

namespace petronilho::platform {

    // ============================================================
    // process_block_v512
    // Aplica transformacao linear: output[i] = input[i] * scale + bias
    // usando FMA em blocos de 16 floats por ciclo.
    //
    // CORRECOES vs versao anterior:
    // - add com 0.0f nao fazia nada, substituido por FMA real
    // - Adicionado tail loop para count nao multiplo de 16
    //
    // Invariante: input e output devem ser 64-byte aligned
    // Garantido pela Arena do Core (alignas(64))
    // ============================================================
    [[gnu::target("avx512f,avx512dq")]]
    void process_block_v512(
        const float* __restrict__ input,
        float*       __restrict__ output,
        size_t                    count,
        float                     scale = 1.0f,
        float                     bias  = 0.0f) noexcept
    {
        const __m512 v_scale = _mm512_set1_ps(scale);
        const __m512 v_bias  = _mm512_set1_ps(bias);

        // Loop principal: 16 floats por iteracao via FMA
        size_t i = 0;
        for (; i + 16 <= count; i += 16) {
            __m512 v = _mm512_load_ps(&input[i]);

            // FMA: (input * scale) + bias em UM ciclo
            // 2x mais rapido que mul seguido de add separados
            v = _mm512_fmadd_ps(v, v_scale, v_bias);

            _mm512_store_ps(&output[i], v);
        }

        // Tail loop com masking AVX512
        // Vantagem do AVX512 sobre AVX2: pode processar
        // elementos restantes com mascara em vez de loop escalar
        if (i < count) {
            const uint16_t mask = (uint16_t)((1u << (count - i)) - 1u);
            __m512 v = _mm512_maskz_load_ps(mask, &input[i]);
            v = _mm512_fmadd_ps(v, v_scale, v_bias);
            _mm512_mask_store_ps(&output[i], mask, v);
        }
    }

    // ============================================================
    // find_match_512 - Busca branchless de 16 chaves
    //
    // ALGORITMO: Hash bucket com resolucao SIMD
    // BASE TEORICA: Cormen Cap.11 - Hash Tables
    // Compara 16 chaves simultaneamente retornando bitmask.
    // Bit i=1 significa keys[i] == target.
    //
    // Uso tipico no Core: verificar se um pacote pertence
    // a um grupo de 16 IDs monitorados em O(1).
    //
    // Retorno: bitmask de 16 bits onde bit i indica match
    // Complexidade: O(1) para 16 chaves
    //
    // Invariante: keys deve apontar para 16 uint32_t
    //             alinhados a 64 bytes
    // ============================================================
    [[gnu::target("avx512f")]]
    [[nodiscard]]
    uint16_t find_match_512(
        const uint32_t* __restrict__ keys,
        uint32_t                     target) noexcept
    {
        const __m512i v_keys   = _mm512_load_si512(
            reinterpret_cast<const __m512i*>(keys));
        const __m512i v_target = _mm512_set1_epi32(
            static_cast<int>(target));

        // Compara 16 inteiros simultaneamente
        // Retorna mascara de 16 bits: bit i=1 se keys[i]==target
        return static_cast<uint16_t>(
            _mm512_cmpeq_epi32_mask(v_keys, v_target));
    }

} // namespace petronilho::platform