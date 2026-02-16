#pragma once
#include "core/sys/arena_atomic.hpp" // Usa a arena escalável
#include "core/sys/handle.hpp"

namespace super_core::relational {

template<typename K, typename V>
struct HashEntry {
    K key;
    sys::Handle<V> value_handle;
    bool occupied = false;
};

template<typename K, typename V>
class HashTable {
private:
    HashEntry<K, V>* table;
    size_t capacity;

    size_t hash(K key) const {
        return static_cast<size_t>(key) % capacity;
    }

public:
    // Ajustado para aceitar ScalableArena
    HashTable(sys::ScalableArena& arena, size_t max_entries) : capacity(max_entries) {
        auto h = arena.allocate<HashEntry<K, V>>(max_entries);
        table = h.get_ptr();
        for(size_t i = 0; i < capacity; ++i) table[i].occupied = false;
    }

    void insert(K key, sys::Handle<V> handle) {
        size_t h = hash(key);
        while (table[h].occupied) {
            if (table[h].key == key) break;
            h = (h + 1) % capacity;
        }
        table[h].key = key;
        table[h].value_handle = handle;
        table[h].occupied = true;
    }

    sys::Handle<V> get(K key) const {
        size_t h = hash(key);
        size_t start = h;
        while (table[h].occupied) {
            if (table[h].key == key) return table[h].value_handle;
            h = (h + 1) % capacity;
            if (h == start) break;
        }
        return sys::Handle<V>{0xFFFFFFFF};
    }
};

} // namespace super_core::relational
