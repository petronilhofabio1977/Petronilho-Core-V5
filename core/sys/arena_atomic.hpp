#pragma once
#include <cstdint>
#include "core/sys/handle.hpp"

namespace super_core::sys {

class ScalableArena {
private:
    uint8_t* base;
    size_t capacity;
    volatile size_t global_offset; // Volatile + Intrinsics

public:
    ScalableArena(void* buffer, size_t size) 
        : base(reinterpret_cast<uint8_t*>(buffer)), 
          capacity(size), 
          global_offset(0) {}

    template<typename T>
    Handle<T> allocate(size_t count = 1) {
        size_t size_needed = sizeof(T) * count;
        // Uso de intrínseco legados que funcionam em qualquer x64
        size_t offset = __sync_fetch_and_add(&global_offset, size_needed);
        
        if (offset + size_needed > capacity) return Handle<T>{0xFFFFFFFF};
        return Handle<T>{ .index = static_cast<uint32_t>(offset) };
    }
};
}
