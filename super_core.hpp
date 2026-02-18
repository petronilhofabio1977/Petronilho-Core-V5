#ifndef SUPER_CORE_HPP
#define SUPER_CORE_HPP

#include <iostream>
#include <atomic>
#include <chrono>
#include <sys/mman.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdexcept>
#include <cstdint>

struct RecordHeader {
    uint64_t timestamp_ns; 
    uint32_t payload_len;  
    uint32_t magic_id;     
};

namespace petronilho {

class UniversalArena {
    uint8_t* memory;
    size_t capacity;
    int fd;
    alignas(64) std::atomic<size_t> offset;

public:
    UniversalArena(const char* path, size_t size) : offset(0) {
        fd = open(path, O_RDWR | O_CREAT | O_TRUNC, 0666);
        if (fd < 0) throw std::runtime_error("Erro ao criar arquivo de Journal.");
        if (ftruncate(fd, size) != 0) throw std::runtime_error("Erro de redimensionamento.");

        memory = (uint8_t*)mmap(nullptr, size, PROT_READ | PROT_WRITE, 
                                MAP_SHARED | MAP_POPULATE, fd, 0);

        if (memory == MAP_FAILED) {
            memory = (uint8_t*)mmap(nullptr, size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
            if (memory == MAP_FAILED) throw std::runtime_error("Falha critica de mmap.");
        }
        capacity = size;
    }

    ~UniversalArena() {
        if (memory) munmap(memory, capacity);
        if (fd >= 0) close(fd);
    }

    inline void* allocate(size_t data_size, RecordHeader** out_header) {
        size_t total_size = sizeof(RecordHeader) + data_size;
        total_size = (total_size + 63) & ~static_cast<size_t>(63);
        size_t current = offset.fetch_add(total_size, std::memory_order_relaxed);
        if (current + total_size > capacity) return nullptr;
        uint8_t* block = memory + current;
        *out_header = reinterpret_cast<RecordHeader*>(block);
        return block + sizeof(RecordHeader);
    }

    void reset() { offset.store(0, std::memory_order_release); }
    size_t get_offset() { return offset.load(std::memory_order_acquire); }
};
} 
#endif
