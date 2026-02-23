#ifndef RING_BUFFER_HPP
#define RING_BUFFER_HPP

#include <cstdint>
#include <cstring>
#include <atomic>
#include <string>
#include <stdexcept>
#include <sys/mman.h>
#include <fcntl.h>
#include <unistd.h>

namespace petronilho {

struct alignas(64) RingSlot {
    uint64_t timestamp_ns;
    uint32_t latency_ns;
    uint32_t packet_id;
    uint32_t payload_len;
    uint8_t  _pad[4];
    uint8_t  payload[40];
};

static_assert(sizeof(RingSlot) == 64, "RingSlot deve ter 64 bytes");

class RingBuffer {
private:
    RingSlot*            m_slots;
    size_t               m_capacity;
    size_t               m_mask;
    std::atomic<size_t>  m_write_idx;
    std::atomic<size_t>  m_read_idx;
    int                  m_fd;

public:
    RingBuffer(const std::string& filename, size_t capacity)
        : m_capacity(capacity), m_mask(capacity - 1)
        , m_write_idx(0), m_read_idx(0), m_fd(-1)
    {
        if ((capacity & (capacity - 1)) != 0)
            throw std::runtime_error("Capacidade deve ser potencia de 2");

        size_t file_size = capacity * sizeof(RingSlot);
        m_fd = open(filename.c_str(), O_RDWR | O_CREAT | O_TRUNC, 0666);
        if (m_fd < 0) throw std::runtime_error("Erro ao abrir arquivo");
        if (ftruncate(m_fd, (off_t)file_size) != 0)
            throw std::runtime_error("Erro ao dimensionar arquivo");
        m_slots = (RingSlot*)mmap(NULL, file_size,
                                   PROT_READ | PROT_WRITE,
                                   MAP_SHARED, m_fd, 0);
        if (m_slots == MAP_FAILED)
            throw std::runtime_error("Erro no mmap");
    }

    ~RingBuffer() {
        if (m_slots && m_slots != MAP_FAILED)
            munmap(m_slots, m_capacity * sizeof(RingSlot));
        if (m_fd >= 0) close(m_fd);
    }

    bool write(uint64_t ts_ns, uint32_t lat_ns,
               uint32_t pkt_id, const void* data, uint32_t len) noexcept
    {
        size_t w = m_write_idx.load(std::memory_order_relaxed);
        size_t r = m_read_idx.load(std::memory_order_acquire);
        if ((w - r) >= m_capacity) return false;
        RingSlot& slot    = m_slots[w & m_mask];
        slot.timestamp_ns = ts_ns;
        slot.latency_ns   = lat_ns;
        slot.packet_id    = pkt_id;
        slot.payload_len  = (len < 40) ? len : 40;
        if (data && len > 0)
            std::memcpy(slot.payload, data, slot.payload_len);
        m_write_idx.store(w + 1, std::memory_order_release);
        return true;
    }

    bool read(RingSlot& out) noexcept {
        size_t r = m_read_idx.load(std::memory_order_relaxed);
        size_t w = m_write_idx.load(std::memory_order_acquire);
        if (r == w) return false;
        out = m_slots[r & m_mask];
        m_read_idx.store(r + 1, std::memory_order_release);
        return true;
    }

    size_t size()     const noexcept {
        return m_write_idx.load(std::memory_order_relaxed)
             - m_read_idx.load(std::memory_order_relaxed);
    }
    size_t capacity() const noexcept { return m_capacity; }
    bool   empty()    const noexcept { return size() == 0; }
};

} // namespace petronilho
#endif
