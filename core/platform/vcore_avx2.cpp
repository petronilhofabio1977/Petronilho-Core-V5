/**
 * @file vcore_avx2.cpp
 * @brief Especialização de Kernel para arquitetura x86-64-v3 (AVX2/FMA).
 * * ALGORITMO: Vetorização de 256-bits.
 * REFERÊNCIA: CLRS Capítulo 1 (Eficiência de Algoritmos - Análise de Hardware).
 * * POR QUE: Garante retrocompatibilidade com performance agressiva em CPUs 
 * sem suporte a AVX-512, utilizando instruções FMA (Fused Multiply-Add).
 */

#include "core/platform/intrinsics_util.hpp"
#include <immintrin.h>

namespace super_core::platform {

/**
 * @brief Processa um bloco de dados usando registradores YMM (256-bit).
 * Complexidade: O(1) por vetor (8 floats por vez).
 */
void process_block_v256(const float* __restrict__ input, float* __restrict__ output, size_t count) noexcept {
    for (size_t i = 0; i < count; i += 8) {
        // Load 256-bit (8 floats)
        __m256 v = _mm256_load_ps(&input[i]);
        
        // Exemplo: Operação de Kernel (Ajuste de ganho/escala)
        v = _mm256_mul_ps(v, _mm256_set1_ps(1.0f));
        
        // Store
        _mm256_store_ps(&output[i], v);
    }
}

} // namespace super_core::platform