// Layer: L0 | Version: 1.0.0 | Author: Fabio Petronilho de Oliveira
//
// ================================================================
// platform_detect.hpp - Deteccao de Plataforma e OS
// ================================================================
//
// O QUE ESSE CODIGO FAZ:
// Define macros que permitem ao resto do Core saber em qual
// sistema operacional e arquitetura esta rodando, sem if/else
// espalhados pelo codigo. Centraliza toda a logica de
// compatibilidade em um unico lugar.
//
// ALGORITMO: Tabela de Dispatch por Macros
// BASE TEORICA: Cormen Cap.5 - Probabilistic Analysis
// A selecao de plataforma e deterministica em compile-time,
// equivalente a uma lookup table O(1) de Cormen Cap.11,
// mas resolvida pelo preprocessador antes da execucao.
// ================================================================

#pragma once

// ============================================================
// Deteccao de Sistema Operacional
// ============================================================
#if defined(_WIN32) || defined(_WIN64)
    #define PETRONILHO_OS_WINDOWS 1
    #define PETRONILHO_OS_NAME "Windows"
#elif defined(__linux__)
    #define PETRONILHO_OS_LINUX 1
    #define PETRONILHO_OS_NAME "Linux"
#elif defined(__APPLE__)
    #define PETRONILHO_OS_MACOS 1
    #define PETRONILHO_OS_NAME "macOS"
#else
    #define PETRONILHO_OS_UNKNOWN 1
    #define PETRONILHO_OS_NAME "Unknown"
#endif

// ============================================================
// Deteccao de Arquitetura
// ============================================================
#if defined(__x86_64__) || defined(_M_X64)
    #define PETRONILHO_ARCH_X64 1
    #define PETRONILHO_ARCH_NAME "x86_64"
#elif defined(__i386__) || defined(_M_IX86)
    #define PETRONILHO_ARCH_X86 1
    #define PETRONILHO_ARCH_NAME "x86"
#elif defined(__aarch64__) || defined(_M_ARM64)
    #define PETRONILHO_ARCH_ARM64 1
    #define PETRONILHO_ARCH_NAME "ARM64"
#else
    #define PETRONILHO_ARCH_UNKNOWN 1
    #define PETRONILHO_ARCH_NAME "Unknown"
#endif

// ============================================================
// Deteccao de Compilador
// ============================================================
#if defined(_MSC_VER)
    #define PETRONILHO_COMPILER_MSVC 1
    #define PETRONILHO_FORCE_INLINE __forceinline
    #define PETRONILHO_NO_INLINE    __declspec(noinline)
    #define PETRONILHO_RESTRICT     __restrict
#elif defined(__GNUC__) || defined(__clang__)
    #define PETRONILHO_COMPILER_GCC 1
    #define PETRONILHO_FORCE_INLINE __attribute__((always_inline)) inline
    #define PETRONILHO_NO_INLINE    __attribute__((noinline))
    #define PETRONILHO_RESTRICT     __restrict__
#else
    #define PETRONILHO_FORCE_INLINE inline
    #define PETRONILHO_NO_INLINE
    #define PETRONILHO_RESTRICT
#endif

// ============================================================
// Includes condicionais por plataforma
// ============================================================
#ifdef PETRONILHO_OS_WINDOWS
    #define WIN32_LEAN_AND_MEAN
    #define NOMINMAX
    #include <windows.h>
    #include <intrin.h>
#else
    #include <cpuid.h>
    #include <x86intrin.h>
    #include <sched.h>
#endif

// ============================================================
// Cache line size (64 bytes em x86, 128 em ARM Apple Silicon)
// ============================================================
#ifdef PETRONILHO_ARCH_ARM64
    #define PETRONILHO_CACHE_LINE 128
#else
    #define PETRONILHO_CACHE_LINE 64
#endif