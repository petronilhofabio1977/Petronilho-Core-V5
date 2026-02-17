#pragma once
#include <cstdint>
#include <x86intrin.h>

namespace super_core::sys {
    // Captura os ciclos de CPU exatos
    static inline uint64_t read_tsc() {
        return __rdtsc();
    }
}
