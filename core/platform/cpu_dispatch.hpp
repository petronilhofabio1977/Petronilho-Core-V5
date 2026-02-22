// Layer: L0 | Version: 1.1.0 | Author: Fabio Petronilho de Oliveira
//
// ================================================================
// cpu_dispatch.hpp - Deteccao de ISA em tempo de execucao
// ================================================================
//
// O QUE ESSE CODIGO FAZ:
// Detecta em tempo de execucao qual conjunto de instrucoes
// SIMD o processador suporta (SSE4, AVX2, AVX512).
// Permite que o Core selecione o path otimizado para o
// hardware disponivel sem recompilar.
//
// ALGORITMO: Arvore de Decisao por CPUID
// BASE TEORICA: Cormen Cap.12 - Binary Search Trees
// A deteccao segue uma arvore de decisao onde cada no
// e uma verificacao de bit de CPUID. A ordem importa:
// verifica do mais capaz (AVX512) para o menos (GENERIC),
// equivalente a busca do maior elemento primeiro.
// Complexidade: O(1) - numero fixo de verificacoes
//
// NOTA SOBRE SEU HARDWARE:
// Intel i7-620M suporta SSE4.2 mas NAO AVX2 nem AVX512.
// O detector retornara ISA::SSE4 no seu PC.
// ================================================================

#pragma once
#include <cpuid.h>
#include <cstdint>

namespace petronilho::platform {

    // Niveis de ISA suportados em ordem crescente de capacidade
    enum class ISA : uint8_t {
        GENERIC = 0,  // Sem SIMD - fallback seguro
        SSE4    = 1,  // SSE4.2 - seu i7-620M
        AVX2    = 2,  // AVX2   - Haswell+
        AVX512  = 3   // AVX512 - Skylake-X+
    };

    // ============================================================
    // detect_isa() - Deteccao via arvore de decisao CPUID
    //
    // CORRECAO vs versao anterior:
    // AVX512 e AVX2 requerem __get_cpuid_count(7, 0, ...)
    // com subleaf=0. __get_cpuid(7,...) nao passa subleaf
    // e pode retornar bits errados em alguns compiladores.
    //
    // Cormen Cap.12: arvore de busca binaria onde a ordem
    // de verificacao define qual caminho e percorrido.
    // ============================================================
    [[nodiscard]]
    inline ISA detect_isa() noexcept {
        uint32_t eax, ebx, ecx, edx;

        // Verifica suporte ao leaf 7 (Extended Features)
        uint32_t max_leaf = __get_cpuid_max(0, nullptr);
        if (max_leaf >= 7) {
            // Leaf 7, Subleaf 0 - Extended Feature Flags
            // CORRECAO: usa __get_cpuid_count com subleaf=0
            __get_cpuid_count(7, 0, &eax, &ebx, &ecx, &edx);

            // AVX512F: EBX bit 16
            if (ebx & (1u << 16)) return ISA::AVX512;

            // AVX2: EBX bit 5
            if (ebx & (1u << 5))  return ISA::AVX2;
        }

        // SSE4.2: ECX bit 20 do leaf 1
        if (__get_cpuid(1, &eax, &ebx, &ecx, &edx)) {
            if (ecx & (1u << 20)) return ISA::SSE4;
        }

        return ISA::GENERIC;
    }

    // Retorna nome legivel do ISA para logs
    [[nodiscard]]
    inline const char* isa_name(ISA isa) noexcept {
        switch (isa) {
            case ISA::AVX512:  return "AVX512";
            case ISA::AVX2:    return "AVX2";
            case ISA::SSE4:    return "SSE4.2";
            default:           return "GENERIC";
        }
    }

} // namespace petronilho::platform