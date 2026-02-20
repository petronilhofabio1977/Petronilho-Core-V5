#ifndef SUPER_CORE_HPP
#define SUPER_CORE_HPP

#include <iostream>
#include <string>
#include <cstdint>
#include <stdexcept>
#include <atomic>

// --- TRATAMENTO MULTIPLATAFORMA DE MEMÓRIA ---
#ifdef _WIN32
    #include <windows.h>
#else
    #include <sys/mman.h>
    #include <sys/stat.h>
    #include <fcntl.h>
    #include <unistd.h>
#endif

struct RecordHeader {
    uint64_t timestamp_ns;
    uint32_t payload_len;
    uint32_t magic_id;
};

namespace petronilho {

class UniversalArena {
private:
    void* m_ptr;
    size_t m_size;
    std::atomic<size_t> m_offset;

#ifdef _WIN32
    HANDLE m_file = INVALID_HANDLE_VALUE;
    HANDLE m_map = NULL;
#else
    int m_fd = -1;
#endif

public:
    UniversalArena(const std::string& filename, size_t size) : m_size(size), m_offset(0) {
#ifdef _WIN32
        // Implementação Windows: CreateFile + CreateFileMapping + MapViewOfFile
        m_file = CreateFileA(filename.c_str(), GENERIC_READ | GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
        if (m_file == INVALID_HANDLE_VALUE) throw std::runtime_error("Erro ao criar arquivo no Windows");

        m_map = CreateFileMapping(m_file, NULL, PAGE_READWRITE, (DWORD)(size >> 32), (DWORD)(size & 0xFFFFFFFF), NULL);
        if (!m_map) { CloseHandle(m_file); throw std::runtime_error("Erro ao criar mapeamento no Windows"); }

        m_ptr = MapViewOfFile(m_map, FILE_MAP_ALL_ACCESS, 0, 0, size);
        if (!m_ptr) { CloseHandle(m_map); CloseHandle(m_file); throw std::runtime_error("Erro ao mapear visão no Windows"); }
#else
        // Implementação Linux: open + ftruncate + mmap
        m_fd = open(filename.c_str(), O_RDWR | O_CREAT | O_TRUNC, 0666);
        if (m_fd < 0) throw std::runtime_error("Erro ao abrir arquivo no Linux");
        if (ftruncate(m_fd, size) != 0) throw std::runtime_error("Erro ao redimensionar arquivo");
        
        m_ptr = mmap(NULL, size, PROT_READ | PROT_WRITE, MAP_SHARED, m_fd, 0);
        if (m_ptr == MAP_FAILED) throw std::runtime_error("Erro no mmap");
#endif
    }

    ~UniversalArena() {
#ifdef _WIN32
        if (m_ptr) UnmapViewOfFile(m_ptr);
        if (m_map) CloseHandle(m_map);
        if (m_file != INVALID_HANDLE_VALUE) CloseHandle(m_file);
#else
        if (m_ptr) munmap(m_ptr, m_size);
        if (m_fd >= 0) close(m_fd);
#endif
    }

    void* allocate(size_t size, RecordHeader** out_header) {
        size_t total_size = size + sizeof(RecordHeader);
        size_t current_offset = m_offset.fetch_add(total_size);

        if (current_offset + total_size > m_size) return nullptr;

        uint8_t* base = static_cast<uint8_t*>(m_ptr) + current_offset;
        *out_header = reinterpret_cast<RecordHeader*>(base);
        return base + sizeof(RecordHeader);
    }
};

} // namespace petronilho

#endif