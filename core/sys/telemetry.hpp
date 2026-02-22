// Layer: L0 | Version: 1.1.0 | Author: Fabio Petronilho de Oliveira
//
// ================================================================
// telemetry.hpp - Leitura de Contador de Ciclos de CPU
// ================================================================
//
// O QUE ESSE CODIGO FAZ:
// Fornece leitura do TSC (Time Stamp Counter) portavel
// entre Linux e Windows para medicao de latencia com
// precisao de ciclos de CPU.
//
// ALGORITMO: Medicao de Custo de Algoritmo
// BASE TEORICA: Cormen Cap.1 - Sec 1.2 (Analyzing Algorithms)
// Cormen define que analisar um algoritmo significa prever
// os recursos que ele consome. O TSC e a ferramenta que
// implementa essa analise em hardware real, contando
// ciclos exatos consumidos por cada operacao.
// Complexidade da leitura: O(1) - instrucao unica de CPU
//
// CORRECAO vs versao anterior:
// - x86intrin.h substituido por platform_detect.hpp
// - Adicionada barreira de serializacao via CPUID
//   Sem barreira o CPU pode reordenar RDTSC (out-of-order)
//   e a medicao fica imprecisa
// - Adicionado rdtsc_end com RDTSCP para medicao de fim
// ================================================================

#pragma once
#include "core/platform/platform_detect.hpp"
#include "core/platform/timer_util.hpp"
#include <cstdint>

namespace petronilho::sys {

    // ============================================================
    // read_tsc - Leitura simples do TSC sem barreira
    // Use apenas quando a ordem exata nao importa.
    // Para medicao precisa use rdtsc_start/rdtsc_end
    // de timer_util.hpp que incluem barreiras CPUID.
    //
    // Cormen Cap.1: custo O(1) por leitura
    // ============================================================
    [[nodiscard]]
    PETRONILHO_FORCE_INLINE uint64_t read_tsc() noexcept {
    #ifdef PETRONILHO_OS_WINDOWS
        return __rdtsc();
    #else
        return __rdtsc();
    #endif
    }

    // ============================================================
    // Aliases para medicao precisa de inicio e fim
    // Delegam para timer_util.hpp que tem barreiras corretas
    //
    // Uso correto:
    //   uint64_t t1 = tsc_begin();
    //   // codigo a medir
    //   uint64_t t2 = tsc_end();
    //   uint64_t ciclos = t2 - t1;
    // ============================================================
    [[nodiscard]]
    PETRONILHO_FORCE_INLINE uint64_t tsc_begin() noexcept {
        return petronilho::platform::rdtsc_start();
    }

    [[nodiscard]]
    PETRONILHO_FORCE_INLINE uint64_t tsc_end() noexcept {
        return petronilho::platform::rdtsc_end();
    }

} // namespace petronilho::sys