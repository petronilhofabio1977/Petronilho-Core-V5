// Layer: L1 | Version: 1.1.0 | Author: Fabio Petronilho de Oliveira
//
// ================================================================
// vcore_sse4.cpp - Kernel Vetorial SSE4.2 128 bits
// ================================================================
//
// O QUE ESSE CODIGO FAZ:
// Processa blocos de floats usando registradores XMM de 128 bits.
// Cada instrucao opera em 4 floats simultaneamente.
// Este e o path usado no seu Intel i7-620M (Westmere, 2010).
//
// ALGORITMO: Vetorizacao de Loop com SSE4.2
// BASE TEORICA: Cormen Cap.27 - Multithreaded Algorithms
// Cormen Cap.27 trata de paralelismo de tarefas (threads).
// SSE4 implementa paralelismo de dados dentro de uma thread:
// ao inves de 4 threads processando 1 float cada, uma thread
// processa 4 floats em um unico ciclo de CPU.
// Complexidade: O(n/4) onde n = count
//
// ESTE ARQUIVO E O MAIS IMPORTANTE DO PROJETO:
// SSE4.2 e o nivel maximo suportado pelo i7-620M.
// AVX2 e AVX512 sao desabilitados em runtime pelo
// cpu_dispatch.hpp nesse hardware.
// ================================================================

#include "core/platform/intrinsics_util.hpp"
#include <nmmintrin.h>
#include <cstddef>

namespace petronilho::platform {

    // ============================================================
    // process_block_v128
    // Aplica transformacao linear: output[i] = input[i] * scale + bias
    // usando SSE4 em blocos de 4 floats por ciclo.
    //
    // CORRECOES vs versao anterior:
    // - add com 0.0f nao fazia nada, substituido por FMA emulado
    // - Adicionado tail loop para count nao multiplo de 4
    // - Adicionado __restrict__ para otimizacao do compilador
    //
    // NOTA: SSE4 nao tem FMA nativo. FMA foi introduzido no
    // Haswell (AVX2). Aqui emulamos com mul + add separados.
    //
    // Invariante: input e output devem ser 16-byte aligned
    // ============================================================
    [[gnu::target("sse4.2")]]
    void process_block_v128(
        const float* __restrict__ input,
        float*       __restrict__ output,
        size_t                    count,
        float                     scale = 1.0f,
        float                     bias  = 0.0f) noexcept
    {
        const __m128 v_scale = _mm_set1_ps(scale);
        const __m128 v_bias  = _mm_set1_ps(bias);

        // Loop principal: 4 floats por iteracao
        size_t i = 0;
        for (; i + 4 <= count; i += 4) {
            __m128 v = _mm_load_ps(&input[i]);

            // SSE4 nao tem FMA: usa mul + add separados
            // AVX2 e AVX512 fazem isso em um unico ciclo
            v = _mm_add_ps(_mm_mul_ps(v, v_scale), v_bias);

            _mm_store_ps(&output[i], v);
        }

        // Tail loop escalar para elementos restantes
        // SSE4 nao tem masking como AVX512
        // CORRECAO: versao anterior ignorava esses elementos
        for (; i < count; ++i) {
            output[i] = input[i] * scale + bias;
        }
    }

} // namespace petronilho::platform