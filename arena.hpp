#ifndef ARENA_HPP
#define ARENA_HPP
#include <atomic>
#include <cstdint>
#include <cstdlib>
#include <new>

namespace petronilho {
class ScalableArena {
    uint8_t* memory;
    size_t capacity;
    alignas(64) std::atomic<size_t> offset;
public:
    ScalableArena(size_t size) : offset(0) {
        const size_t alignment = 4096;
        capacity = (size + alignment - 1) & ~(alignment - 1);
        if (posix_memalign(reinterpret_cast<void**>(&memory), alignment, capacity) != 0) {
            std::abort();
        }
    }
    ~ScalableArena() { std::free(memory); }
    inline void* allocate(size_t size) {
        size_t aligned_req = (size + 15) & ~static_cast<size_t>(15);
        size_t current = offset.fetch_add(aligned_req, std::memory_order_relaxed);
        if (current + aligned_req > capacity) return nullptr;
        return memory + current;
    }
    void reset() { offset.store(0, std::memory_order_relaxed); }
};
}
#endif
