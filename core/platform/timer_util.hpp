// Layer: L0 | Version: 1.0.0 | Author: Fabio Petronilho de Oliveira
//
// ================================================================
// timer_util.hpp - Temporizacao de Alta Resolucao Portatil
// ================================================================
//
// O QUE ESSE CODIGO FAZ:
// Fornece RDTSC e timestamp em nanosegundos funcionando
// identicamente no Linux e Windows sem mudar o codigo
// que usa essas funcoes.
//
// ALGORITMO: Abstracao de Hardware via Wrapper
// BASE TEORICA: Cormen Cap.1 - modelo de custo uniforme
// O modelo de custo do Cormen assume que operacoes basicas
// tem custo O(1). Este arquivo garante que a medicao de
// tempo seja O(1) em qualquer plataforma.
// ================================================================

#pragma once
#include "platform_detect.hpp"
#include <cstdint>

#ifdef PETRONILHO_OS_WINDOWS
    #include <intrin.h>
#else
    #include <x86intrin.h>
#endif

namespace petronilho::platform {

    // ============================================================
    // rdtsc_start / rdtsc_end
    // Portateis entre Linux (GCC/Clang) e Windows (MSVC)
    // ============================================================
    [[nodiscard]]
    PETRONILHO_FORCE_INLINE uint64_t rdtsc_start() noexcept {
    #ifdef PETRONILHO_COMPILER_MSVC
        _ReadWriteBarrier();
        return __rdtsc();
    #else
        uint32_t lo, hi;
        asm volatile (
            "cpuid\n\t"
            "rdtsc"
            : "=a"(lo), "=d"(hi)
            :: "rbx", "rcx", "memory"
        );
        return (uint64_t)hi << 32 | lo;
    #endif
    }

    [[nodiscard]]
    PETRONILHO_FORCE_INLINE uint64_t rdtsc_end() noexcept {
    #ifdef PETRONILHO_COMPILER_MSVC
        unsigned int aux;
        uint64_t t = __rdtscp(&aux);
        _ReadWriteBarrier();
        return t;
    #else
        uint32_t lo, hi, aux;
        asm volatile (
            "rdtscp"
            : "=a"(lo), "=d"(hi), "=c"(aux)
            :: "memory"
        );
        asm volatile ("cpuid" ::: "rax","rbx","rcx","rdx","memory");
        return (uint64_t)hi << 32 | lo;
    #endif
    }

    // ============================================================
    // now_ns - Timestamp em nanosegundos portatil
    // ============================================================
    [[nodiscard]]
    inline uint64_t now_ns() noexcept {
    #ifdef PETRONILHO_OS_WINDOWS
        LARGE_INTEGER freq, count;
        QueryPerformanceFrequency(&freq);
        QueryPerformanceCounter(&count);
        return (uint64_t)(count.QuadPart * 1000000000LL / freq.QuadPart);
    #else
        struct timespec ts;
        clock_gettime(CLOCK_MONOTONIC_RAW, &ts);
        return (uint64_t)ts.tv_sec * 1000000000ULL + ts.tv_nsec;
    #endif
    }

} // namespace petronilho::platform