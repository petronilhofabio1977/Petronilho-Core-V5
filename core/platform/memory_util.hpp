// Layer: L0 | Version: 1.0.0 | Author: Fabio Petronilho de Oliveira
//
// ================================================================
// memory_util.hpp - Alocacao Alinhada Portatil
// ================================================================
//
// O QUE ESSE CODIGO FAZ:
// Substitui posix_memalign (Linux only) por uma abstracao
// que funciona no Windows via _aligned_malloc e no Linux
// via posix_memalign. A Arena usa esse arquivo.
//
// ALGORITMO: Alocacao Alinhada O(1)
// BASE TEORICA: Cormen Cap.17 - Analise Amortizada
// Mesma base da Arena: alocacao com alinhamento garantido
// para evitar penalidade de acesso nao alinhado em SIMD.
// ================================================================

#pragma once
#include "platform_detect.hpp"
#include <cstddef>
#include <cstdlib>

#ifdef PETRONILHO_OS_WINDOWS
    #include <malloc.h>
#else
    #include <cstdlib>
#endif

namespace petronilho::platform {

    // ============================================================
    // aligned_alloc_portable
    // Aloca 'size' bytes com alinhamento 'alignment'.
    // alignment deve ser potencia de 2.
    // Retorna nullptr em falha.
    // ============================================================
    [[nodiscard]]
    inline void* aligned_alloc_portable(
        size_t alignment,
        size_t size) noexcept
    {
    #ifdef PETRONILHO_OS_WINDOWS
        return _aligned_malloc(size, alignment);
    #else
        void* ptr = nullptr;
        if (posix_memalign(&ptr, alignment, size) != 0)
            return nullptr;
        return ptr;
    #endif
    }

    // ============================================================
    // aligned_free_portable
    // Libera memoria alocada por aligned_alloc_portable.
    // IMPORTANTE: nao use free() comum para memoria alinhada
    // no Windows, pois _aligned_malloc requer _aligned_free.
    // ============================================================
    inline void aligned_free_portable(void* ptr) noexcept {
    #ifdef PETRONILHO_OS_WINDOWS
        _aligned_free(ptr);
    #else
        free(ptr);
    #endif
    }

    // ============================================================
    // cpu_affinity_portable
    // Define afinidade de CPU portavel entre Linux e Windows
    // ============================================================
    inline bool cpu_affinity_portable(int cpu_id) noexcept {
    #ifdef PETRONILHO_OS_WINDOWS
        DWORD_PTR mask = 1ULL << cpu_id;
        return SetThreadAffinityMask(
            GetCurrentThread(), mask) != 0;
    #else
        cpu_set_t cpuset;
        CPU_ZERO(&cpuset);
        CPU_SET(cpu_id, &cpuset);
        return sched_setaffinity(
            0, sizeof(cpu_set_t), &cpuset) == 0;
    #endif
    }

} // namespace petronilho::platform