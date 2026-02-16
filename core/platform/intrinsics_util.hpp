/**
 * @file intrinsics_util.hpp
 * @brief Abstração de Instruções Vetoriais (SIMD).
 * * ALGORITMO: Paralelismo em nível de instrução.
 * REFERÊNCIA: CLRS Apêndice (Eficiência de Algoritmos em Hardware Moderno).
 * * POR QUE: Permite processar múltiplos elementos de dados (ex: 16 floats) em um 
 * único ciclo de CPU, essencial para o throughput de dados do SUPER_CORE.
 */

#pragma once
#include <immintrin.h>

namespace super_core::platform {

// Usamos __attribute__((target)) para garantir que o compilador
// gere o código correto apenas quando chamado por código v4.
struct SIMD_512 {
    [[gnu::target("avx512f")]]
    static inline __m512 load(const float* addr) noexcept {
        return _mm512_load_ps(addr);
    }

    [[gnu::target("avx512f")]]
    static inline void store(float* addr, __m512 reg) noexcept {
        _mm512_store_ps(addr, reg);
    }
};

} // namespace super_core::platform