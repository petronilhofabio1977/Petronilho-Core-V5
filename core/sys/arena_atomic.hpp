#pragma once
#include <atomic>
#include <cstdint>
#include "core/sys/handle.hpp"

namespace super_core::sys {

struct LocalChunk {
    uint32_t current_index;
    uint32_t end_index;
};

static thread_local LocalChunk t_local_chunk = {0, 0};

class ScalableArena {
private:
    uint8_t* base;
    size_t capacity;
    
    alignas(64) std::atomic<size_t> global_offset;
    char padding[64]; 

    const size_t CHUNK_SIZE = 1024 * 128; // Aumentado para 128KB

public:
    ScalableArena(void* buffer, size_t size) 
        : base(reinterpret_cast<uint8_t*>(buffer)), 
          capacity(size), 
          global_offset(0) {}

    template<typename T>
    Handle<T> allocate(size_t count = 1) {
        uint32_t size_needed = static_cast<uint32_t>(sizeof(T) * count);

        // Se o pedido for maior que um chunk inteiro, vai direto pro global
        if (size_needed > CHUNK_SIZE) {
            size_t offset = global_offset.fetch_add(size_needed, std::memory_order_relaxed);
            if (offset + size_needed > capacity) return Handle<T>{0xFFFFFFFF};
            return Handle<T>{ .index = static_cast<uint32_t>(offset) };
        }

        // Caso comum: Alocação via Chunk Local
        if (t_local_chunk.current_index + size_needed > t_local_chunk.end_index) {
            size_t current_global = global_offset.fetch_add(CHUNK_SIZE, std::memory_order_relaxed);
            if (current_global + CHUNK_SIZE > capacity) return Handle<T>{0xFFFFFFFF};

            t_local_chunk.current_index = static_cast<uint32_t>(current_global);
            t_local_chunk.end_index = static_cast<uint32_t>(current_global + CHUNK_SIZE);
        }

        uint32_t index = t_local_chunk.current_index;
        t_local_chunk.current_index += size_needed;
        
        return Handle<T>{ .index = index };
    }
};
}
