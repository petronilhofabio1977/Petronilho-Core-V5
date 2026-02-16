#pragma once
#include <algorithm>
#include "core/sys/arena.hpp"
#include "core/sys/handle.hpp"

namespace super_core::priority {

template<typename T>
class BinaryHeap {
private:
    sys::Handle<T>* data;
    size_t capacity;
    size_t current_size;

    inline size_t parent(size_t i) { return (i - 1) >> 1; }
    inline size_t left(size_t i)   { return (i << 1) + 1; }
    inline size_t right(size_t i)  { return (i << 1) + 2; }

    void min_heapify(size_t i) {
        size_t l = left(i);
        size_t r = right(i);
        size_t smallest = i;
        if (l < current_size && *data[l] < *data[smallest]) smallest = l;
        if (r < current_size && *data[r] < *data[smallest]) smallest = r;
        if (smallest != i) {
            std::swap(data[i], data[smallest]);
            min_heapify(smallest);
        }
    }

public:
    BinaryHeap(sys::Arena& arena, size_t max_nodes) 
        : capacity(max_nodes), current_size(0) {
        auto array_handle = arena.allocate<sys::Handle<T>>(max_nodes);
        data = array_handle.get_ptr();
    }

    void push(sys::Handle<T> node_handle) {
        if (current_size >= capacity) return;
        size_t i = current_size++;
        data[i] = node_handle;
        while (i > 0 && *data[i] < *data[parent(i)]) {
            std::swap(data[i], data[parent(i)]);
            i = parent(i);
        }
    }

    sys::Handle<T> pop() {
        if (current_size == 0) return sys::Handle<T>{0xFFFFFFFF};
        sys::Handle<T> root = data[0];
        data[0] = data[--current_size];
        min_heapify(0);
        return root;
    }

    bool empty() const { return current_size == 0; }
};

} // namespace super_core::priority
