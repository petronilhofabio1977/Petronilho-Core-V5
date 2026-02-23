#ifndef PERSISTENT_ARENA_HPP
#define PERSISTENT_ARENA_HPP

#include <sys/mman.h>
#include <fcntl.h>
#include <unistd.h>
#include <atomic>
#include <iostream>
#include <stdexcept>

namespace petronilho {

class PersistentArena {
    uint8_t* memory;
    size_t capacity;
    int fd;
    alignas(64) std::atomic<size_t> offset;

public:
    PersistentArena(const char* filename, size_t size) : offset(0) {
        // 1. Abrir/Criar arquivo no NVMe (ou disco local)
        fd = open(filename, O_RDWR | O_CREAT | O_TRUNC, 0666);
        if (fd < 0) throw std::runtime_error("Falha ao abrir arquivo");

        // 2. Definir o tamanho do arquivo fisicamente
        if (ftruncate(fd, size) != 0) throw std::runtime_error("Falha ao redimensionar arquivo");

        // 3. Mapeamento Zero-Copy com Blindagem (MAP_POPULATE evita lag de Page Fault)
        memory = (uint8_t*)mmap(nullptr, size, PROT_READ | PROT_WRITE, 
                                MAP_SHARED | MAP_POPULATE, fd, 0);

        if (memory == MAP_FAILED) throw std::runtime_error("mmap falhou");
        capacity = size;
    }

    ~PersistentArena() {
        munmap(memory, capacity);
        close(fd);
    }

    inline void* allocate(size_t size) {
        size_t aligned_req = (size + 63) & ~static_cast<size_t>(63);
        size_t current = offset.fetch_add(aligned_req, std::memory_order_relaxed);
        
        if (current + aligned_req > capacity) return nullptr;
        return memory + current;
    }

    void sync() { msync(memory, capacity, MS_ASYNC); }
    void reset() { offset.store(0, std::memory_order_relaxed); }
};
}
#endif
