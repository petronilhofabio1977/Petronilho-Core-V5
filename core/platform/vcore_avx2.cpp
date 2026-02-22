// Layer: L1 | Version: 1.1.0 | Author: Fabio Petronilho de Oliveira
//
// ================================================================
// vcore_avx2.cpp - Kernel Vetorial AVX2/FMA 256 bits
// ================================================================
//
// O QUE ESSE CODIGO FAZ:
// Processa blocos de floats usando registradores YMM de 256 bits.
// Cada instrucao opera em 8 floats simultaneamente.
// Usa FMA (Fused Multiply-Add) para operacoes compostas em
// um unico ciclo de CPU com menor erro de arredondamento.
//
// ALGORITMO: Vetorizacao de Loop com FMA
// BASE TEORICA: Cormen Cap.1 (modelo de custo de algoritmos)
//               Cormen Cap.28 (operacoes em vetores e matrizes)
// Sem SIMD: O(n) com constante k=1 operacao por ciclo
// Com AVX2: O(n/8) com 8 floats por ciclo
// Com FMA:  O(n/8) mesma complexidade mas menos instrucoes,
//           pois multiply+add vira uma unica instrucao.
//
// FMA: resultado = (a * b) + c em UM ciclo
//      sem FMA  = mul(a,b) seguido de add(resultado, c) = DOIS ciclos
// ================================================================

#include "core/platform/intrinsics_util.hpp"
#include <immintrin.h>
#include <cstddef>

namespace petronilho::platform {

    // ============================================================
    // process_block_v256
    // Aplica transformacao linear: output[i] = input[i] * scale + bias
    // usando FMA em blocos de 8 floats por ciclo.
    //
    // CORRECAO vs versao anterior:
    // - Multiplicar por 1.0f nao fazia nada, substituido por FMA real
    // - Adicionado tail loop para count nao multiplo de 8
    // - Adicionado restrict para permitir otimizacao do compilador
    //
    // Parametros:
    //   input  - ponteiro de entrada, deve ser 32-byte aligned
    //   output - ponteiro de saida, deve ser 32-byte aligned
    //   count  - numero de floats a processar
    //   scale  - fator multiplicador (ganho)
    //   bias   - offset aditivo
    //
    // Complexidade: O(n/8) onde n = count
    // ============================================================
    [[gnu::target("avx2,fma")]]
    void process_block_v256(
        const float* __restrict__ input,
        float*       __restrict__ output,
        size_t                    count,
        float                     scale = 1.0f,
        float                     bias  = 0.0f) noexcept
    {
        // Broadcast scale e bias para todos os 8 lanes do registrador
        const __m256 v_scale = _mm256_set1_ps(scale);
        const __m256 v_bias  = _mm256_set1_ps(bias);

        // Loop principal: processa 8 floats por iteracao
        // FMA: output = input * scale + bias em UM ciclo
        size_t i = 0;
        for (; i + 8 <= count; i += 8) {
            __m256 v = _mm256_load_ps(&input[i]);

            // _mm256_fmadd_ps(a, b, c) = (a * b) + c
            // Cormen Cap.28: operacao vetorial em O(1) por bloco
            v = _mm256_fmadd_ps(v, v_scale, v_bias);

            _mm256_store_ps(&output[i], v);
        }

        // Tail loop: processa elementos restantes escalarmente
        // CORRECAO: versao anterior ignorava esses elementos
        // causando buffer overflow silencioso
        for (; i < count; ++i) {
            output[i] = input[i] * scale + bias;
        }
    }

} // namespace petronilho::platform