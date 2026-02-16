/**
 * @file vcore_sse4.cpp
 * @brief Driver de Performance para Intel Westmere (SSE4.2).
 * ALGORITMO: Vetorização de 128-bits.
 * REFERÊNCIA: CLRS Capítulo 27 (Paralelismo de Dados).
 */
#include <nmmintrin.h> // SSE4.2
#include <cstddef>

namespace super_core::platform {

    // Versão SSE4 da nossa função de processamento
    void process_block_v128(const float* input, float* output, size_t count) noexcept {
        for (size_t i = 0; i < count; i += 4) {
            __m128 v = _mm_load_ps(&input[i]);
            v = _mm_add_ps(v, _mm_set1_ps(0.0f));
            _mm_store_ps(&output[i], v);
        }
    }
}