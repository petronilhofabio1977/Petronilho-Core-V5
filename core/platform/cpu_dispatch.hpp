#pragma once
#include <cpuid.h>

namespace super_core::platform {
    enum class ISA { GENERIC, SSE4, AVX2, AVX512 };

    inline ISA detect_isa() {
        unsigned int eax, ebx, ecx, edx;
        
        // 1. Verificar AVX512 (Nível v4)
        if (__get_cpuid(7, &eax, &ebx, &ecx, &edx) && (ebx & (1 << 16))) return ISA::AVX512;
        
        // 2. Verificar AVX2 (Nível v3)
        if (__get_cpuid(7, &eax, &ebx, &ecx, &edx) && (ebx & (1 << 5))) return ISA::AVX2;
        
        // 3. Verificar SSE4.2 (Nível v2 - O seu Petronilho i7 M 620)
        __get_cpuid(1, &eax, &ebx, &ecx, &edx);
        if (ecx & (1 << 20)) return ISA::SSE4;

        return ISA::GENERIC;
    }
}