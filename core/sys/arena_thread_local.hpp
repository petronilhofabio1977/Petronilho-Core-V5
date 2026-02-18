#ifndef ARENA_THREAD_LOCAL_HPP
#define ARENA_THREAD_LOCAL_HPP

#include <cstddef>
#include <cstdint>
#include <cstdlib>
#include <new>

namespace supercore {

class ThreadLocalArena {

    struct Block {
        uint8_t* memory;
        size_t capacity;
        size_t offset;

        explicit Block(size_t size)
            : capacity(size), offset(0)
        {
            const size_t alignment = 4096;
            size_t aligned =
                (size + alignment - 1) & ~(alignment - 1);

            if (posix_memalign(
                    reinterpret_cast<void**>(&memory),
                    alignment,
                    aligned) != 0)
            {
                std::abort();
            }
        }

        ~Block() { std::free(memory); }

        inline void* allocate(size_t size) {
            size_t aligned =
                (size + 15) & ~static_cast<size_t>(15);

            if (offset + aligned > capacity)
                return nullptr;

            void* ptr = memory + offset;
            offset += aligned;
            return ptr;
        }
    };

    static thread_local Block* local_block;

    static constexpr size_t DEFAULT_SIZE =
        64ULL * 1024 * 1024; // 64MB per thread

public:

    static void initialize(size_t size = DEFAULT_SIZE) {
        if (!local_block)
            local_block = new Block(size);
    }

    static inline void* allocate(size_t size) {
        void* p = local_block->allocate(size);
        if (!p) {
            delete local_block;
            local_block = new Block(DEFAULT_SIZE);
            p = local_block->allocate(size);
        }
        return p;
    }

    static void reset() {
        if (local_block)
            local_block->offset = 0;
    }
};

thread_local ThreadLocalArena::Block*
    ThreadLocalArena::local_block = nullptr;

}

#endif