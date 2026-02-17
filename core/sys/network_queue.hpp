#pragma once
#include <atomic>
#include <cstdint>
#include <cstring>

namespace super_core::net {

template<typename T, size_t Capacity>
struct NetworkQueue {
    std::atomic<size_t> head;
    std::atomic<size_t> tail;
    uint32_t buffer[Capacity];

    NetworkQueue() : head(0), tail(0) {
        std::memset(buffer, 0, sizeof(buffer));
    }

    bool enqueue(uint32_t index) {
        size_t t = tail.load();
        size_t h = head.load();
        if (((t + 1) & (Capacity - 1)) == h) return false;
        buffer[t] = index;
        tail.store((t + 1) & (Capacity - 1));
        return true;
    }

    bool dequeue(uint32_t& out_index) {
        size_t h = head.load();
        size_t t = tail.load();
        if (h == t) return false;
        out_index = buffer[h];
        head.store((h + 1) & (Capacity - 1));
        return true;
    }
};
}
