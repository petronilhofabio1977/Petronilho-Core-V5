#ifndef SUPER_CORE_UNIVERSAL_HPP
#define SUPER_CORE_UNIVERSAL_HPP

#include <iostream>
#include <atomic>
#include <chrono>
#include <cstdint>
#include <stdexcept>

// Detecção de Sistema Operacional para Gerenciamento de Memória
#if defined(_WIN32) || defined(_WIN64)
    #define OS_WINDOWS
    #include <windows.h>
#else
    #define OS_LINUX_UNIX
    #include <sys/mman.h>
    #include <fcntl.h>
    #include <unistd.h>
#endif

struct RecordHeader {
    uint64_t timestamp_ns;
    uint32_t payload_len;
    uint32_t magic;
};

namespace petronilho {

class UniversalArena {
    uint8_t* memory;
    size_t capacity;
    alignas(64) std::atomic<size_t> offset;

#ifdef OS_WINDOWS
    HANDLE hFile;
    HANDLE hMapping;
#else
    int fd;
#endif

public:
    UniversalArena(const char* path, size_t size) : offset(0), capacity(size) {
#ifdef OS_WINDOWS
        hFile = CreateFileA(path, GENERIC_READ | GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
        if (hFile == INVALID_HANDLE_VALUE) throw std::runtime_error("Windows: Erro ao criar arquivo.");
        
        hMapping = CreateFileMapping(hFile, NULL, PAGE_READWRITE, 0, (DWORD)size, NULL);
        if (!hMapping) { CloseHandle(hFile); throw std::runtime_error("Windows: Erro no Mapping."); }
        
        memory = (uint8_t*)MapViewOfFile(hMapping, FILE_MAP_ALL_ACCESS, 0, 0, size);
#else
        fd = open(path, O_RDWR | O_CREAT | O_TRUNC, 0666);
        if (fd < 0) throw std::runtime_error("Linux: Erro ao abrir arquivo.");
        if (ftruncate(fd, size) != 0) throw std::runtime_error("Linux: Erro ftruncate.");
        
        memory = (uint8_t*)mmap(nullptr, size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
#endif
        if (!memory) throw std::runtime_error("Falha critica ao mapear memoria.");
    }

    ~UniversalArena() {
#ifdef OS_WINDOWS
        if (memory) UnmapViewOfFile(memory);
        if (hMapping) CloseHandle(hMapping);
        if (hFile) CloseHandle(hFile);
#else
        if (memory) munmap(memory, capacity);
        if (fd >= 0) close(fd);
#endif
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
};

}
#endif
