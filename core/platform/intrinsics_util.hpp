// Layer: L0 | Version: 1.1.0 | Author: Fabio Petronilho de Oliveira
//
// ================================================================
// intrinsics_util.hpp - Abstracao de Instrucoes Vetoriais SIMD
// ================================================================
//
// O QUE ESSE CODIGO FAZ:
// Fornece wrappers type-safe para instrucoes SIMD dos tres
// niveis suportados pelo Core: SSE4, AVX2 e AVX512.
// Usado em conjunto com cpu_dispatch.hpp para selecionar
// em tempo de execucao o path mais rapido disponivel.
//
// ALGORITMO: Paralelismo em Nivel de Instrucao (ILP)
// BASE TEORICA: Cormen Apendice C e Cap.28 (Matrix Operations)
// SIMD e a implementacao hardware do principio de Cormen
// Cap.28: operacoes em vetores processadas em paralelo.
// SSE4  processa  4 floats por ciclo (128 bits)
// AVX2  processa  8 floats por ciclo (256 bits)
// AVX512 processa 16 floats por ciclo (512 bits)
// Complexidade: O(n/k) onde k = largura do registrador SIMD
//
// NOTA: Seu i7-620M suporta SSE4. Use SIMD_128 na pratica.
// ================================================================

#pragma once
#include <immintrin.h>
#include <cstdint>

namespace petronilho::platform {

    // ============================================================
    // SIMD_128 - SSE4.2 (128 bits = 4 floats por ciclo)
    // Compativel com seu Intel i7-620M
    // ============================================================
    struct SIMD_128 {
        [[gnu::target("sse4.2")]]
        [[nodiscard]]
        static inline __m128 load(const float* addr) noexcept {
            return _mm_load_ps(addr);  // addr deve ser 16-byte aligned
        }

        [[gnu::target("sse4.2")]]
        static inline void store(float* addr, __m128 reg) noexcept {
            _mm_store_ps(addr, reg);   // addr deve ser 16-byte aligned
        }

        [[gnu::target("sse4.2")]]
        [[nodiscard]]
        static inline __m128 add(const float* a,
                                  const float* b) noexcept {
            return _mm_add_ps(_mm_load_ps(a), _mm_load_ps(b));
        }

        // Largura em floats para uso em loops
        static constexpr uint32_t WIDTH = 4;
    };

    // ============================================================
    // SIMD_256 - AVX2 (256 bits = 8 floats por ciclo)
    // Requer Intel Haswell (2013) ou mais novo
    // ============================================================
    struct SIMD_256 {
        [[gnu::target("avx2")]]
        [[nodiscard]]
        static inline __m256 load(const float* addr) noexcept {
            return _mm256_load_ps(addr); // addr deve ser 32-byte aligned
        }

        [[gnu::target("avx2")]]
        static inline void store(float* addr, __m256 reg) noexcept {
            _mm256_store_ps(addr, reg);
        }

        [[gnu::target("avx2")]]
        [[nodiscard]]
        static inline __m256 add(const float* a,
                                  const float* b) noexcept {
            return _mm256_add_ps(_mm256_load_ps(a), _mm256_load_ps(b));
        }

        static constexpr uint32_t WIDTH = 8;
    };

    // ============================================================
    // SIMD_512 - AVX512F (512 bits = 16 floats por ciclo)
    // Requer Intel Skylake-X (2017) ou mais novo
    // ============================================================
    struct SIMD_512 {
        [[gnu::target("avx512f")]]
        [[nodiscard]]
        static inline __m512 load(const float* addr) noexcept {
            return _mm512_load_ps(addr); // addr deve ser 64-byte aligned
        }

        [[gnu::target("avx512f")]]
        static inline void store(float* addr, __m512 reg) noexcept {
            _mm512_store_ps(addr, reg);
        }

        [[gnu::target("avx512f")]]
        [[nodiscard]]
        static inline __m512 add(const float* a,
                                  const float* b) noexcept {
            return _mm512_add_ps(_mm512_load_ps(a), _mm512_load_ps(b));
        }

        static constexpr uint32_t WIDTH = 16;
    };

} // namespace petronilho::platform