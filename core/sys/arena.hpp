#include <atomic>
#include <thread>

namespace super_core::sys {

struct LocalChunk {
    uint32_t current_index;
    uint32_t end_index;
};

// Variável de thread para armazenamento local (Retail)
static thread_local LocalChunk t_local_chunk = {0, 0};

class ScalableArena {
private:
    uint8_t* base;
    size_t capacity;
    std::atomic<size_t> global_offset;
    const size_t CHUNK_SIZE = 1024 * 64; // 64KB por chunk

public:
    ScalableArena(void* buffer, size_t size) 
        : base(reinterpret_cast<uint8_t*>(buffer)), capacity(size), global_offset(0) {}

    template<typename T>
    Handle<T> allocate() {
        size_t size_needed = sizeof(T);

        // Se o chunk local acabou, buscamos um novo no Global (Wholesale)
        if (t_local_chunk.current_index + size_needed > t_local_chunk.end_index) {
            size_t current_global;
            size_t next_global;
            
            do {
                current_global = global_offset.load(std::memory_order_relaxed);
                next_global = current_global + CHUNK_SIZE;
                if (next_global > capacity) return Handle<T>{0xFFFFFFFF};
            } while (!global_offset.compare_exchange_weak(current_global, next_global,
                                                         std::memory_order_acq_rel));

            t_local_chunk.current_index = static_cast<uint32_t>(current_global);
            t_local_chunk.end_index = static_cast<uint32_t>(next_global);
        }

        // Alocação local: Ultra-fast, zero contenção, zero CAS
        uint32_t index = t_local_chunk.current_index;
        t_local_chunk.current_index += size_needed;
        
        return Handle<T>{ .index = index };
    }
};
}