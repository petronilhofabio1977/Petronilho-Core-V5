#pragma once
#include <cstdint>

namespace super_core::sys {

extern uint8_t* G_POOL_BASE;

template<typename T>
struct Handle {
    uint32_t index;

    T* get_ptr() const {
        if (index == 0xFFFFFFFF) return nullptr;
        return reinterpret_cast<T*>(G_POOL_BASE + index);
    }

    T& operator*() const { return *get_ptr(); }
    T* operator->() const { return get_ptr(); }

    bool operator<(const Handle& other) const {
        return (*get_ptr()) < (*other.get_ptr());
    }
    
    bool is_null() const { return index == 0xFFFFFFFF; }
};

} // namespace super_core::sys
