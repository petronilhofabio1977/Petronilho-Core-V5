#pragma once
#include <cstdlib>

#ifdef _WIN32
    #include <malloc.h>
#endif

namespace super_core::sys {

inline void* aligned_alloc_memory(size_t alignment, size_t size) {
    void* ptr = nullptr;
#ifdef _WIN32
    // No Windows (MSVC/MinGW), usamos _aligned_malloc
    ptr = _aligned_malloc(size, alignment);
#else
    // No Linux/Unix, usamos posix_memalign
    if (posix_memalign(&ptr, alignment, size) != 0) {
        return nullptr;
    }
#endif
    return ptr;
}

inline void aligned_free_memory(void* ptr) {
#ifdef _WIN32
    _aligned_free(ptr);
#else
    free(ptr);
#endif
}

} // namespace super_core::sys
