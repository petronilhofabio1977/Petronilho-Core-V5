// Layer: L0 | Version: 2.2.0 | Author: Fabio Petronilho de Oliveira
//
// ================================================================
// arena.hpp - Arena Unificada v2.2
// ================================================================
//
// MELHORIA v2.2 vs v2.1:
// TLS_CHUNK adaptativo em vez de fixo.
// Motivo: objeto maior que 64KB causava refill a cada alocacao,
// perdendo todo o beneficio do cache local.
//
// Exemplo do problema v2.1:
// Objeto de 128KB → TLS_CHUNK = 64KB
// Cada alocacao → refill → fetch_add atomico
// Beneficio do TLS = zero
//
// Solucao v2.2:
// Objeto de 128KB → chunk = 256KB (objeto * 2)
// Thread faz 2 alocacoes locais antes de ir ao global
// Beneficio do TLS restaurado
//
// HISTORICO:
// v2.0: unificou arena_atomic + arena_thread_local
// v2.1: alignof(T) em vez de alinhamento fixo 16
// v2.2: TLS_CHUNK adaptativo para objetos grandes
//
// ALGORITMO: Bump Allocator Hierarquico
// BASE TEORICA: Cormen Cap.17 Sec.17.1 - Aggregate Analysis
// ================================================================

#pragma once
#include "handle.hpp"
#include "core/platform/memory_util.hpp"
#include <atomic>
#include <cstdint>
#include <cstddef>

namespace petronilho::sys {

    struct ArenaConfig {
        // Tamanho minimo do chunk TLS
        // Para objetos maiores o chunk se adapta
        static constexpr size_t TLS_CHUNK_MIN  = 64  * 1024; // 64KB
        static constexpr size_t TLS_CHUNK_MAX  = 256 * 1024; // 256KB teto
        static constexpr size_t BASE_ALIGNMENT = 64;         // AVX512

        // Calcula tamanho ideal do chunk para um objeto
        // Se objeto cabe em 64KB usa 64KB
        // Se objeto e maior usa objeto * 2 ate o teto de 256KB
        static constexpr size_t chunk_for(size_t obj_size) noexcept {
            if (obj_size <= TLS_CHUNK_MIN)
                return TLS_CHUNK_MIN;
            const size_t ideal = obj_size * 2;
            return ideal < TLS_CHUNK_MAX ? ideal : TLS_CHUNK_MAX;
        }
    };

    class ScalableArena {
    private:
        uint8_t*                        m_base;
        size_t                          m_capacity;
        alignas(64) std::atomic<size_t> m_offset;

        struct ThreadChunk {
            uint8_t* base = nullptr;
            size_t   used = 0;
            size_t   size = 0;

            bool has_space(size_t need) const noexcept {
                return base && (used + need <= size);
            }
        };

        static thread_local ThreadChunk t_chunk;

        // ============================================================
        // refill_chunk - pede novo chunk do global
        //
        // MELHORIA v2.2: chunk_size adaptativo via ArenaConfig::chunk_for
        // Evita refill por alocacao em objetos grandes
        //
        // Cormen Cap.17: custo do refill diluido entre
        // todas as alocacoes locais do chunk.
        // ============================================================
        bool refill_chunk(size_t obj_size) noexcept {
            const size_t chunk_size = ArenaConfig::chunk_for(obj_size);

            const size_t off = m_offset.fetch_add(
                chunk_size, std::memory_order_relaxed);

            if (off + chunk_size > m_capacity) return false;

            t_chunk.base = m_base + off;
            t_chunk.used = 0;
            t_chunk.size = chunk_size;
            return true;
        }

    public:
        ScalableArena(void* buffer, size_t size) noexcept
            : m_base(static_cast<uint8_t*>(buffer))
            , m_capacity(size)
            , m_offset(0)
        {}

        ScalableArena(const ScalableArena&)            = delete;
        ScalableArena& operator=(const ScalableArena&) = delete;

        static void initialize_thread() noexcept {
            t_chunk = {};
        }

        template<typename T>
        [[nodiscard]]
        Handle<T> allocate(size_t count = 1) noexcept {
            const size_t raw       = sizeof(T) * count;
            const size_t alignment = alignof(T);
            const size_t aligned   =
                (raw + (alignment - 1)) & ~(alignment - 1);

            if (t_chunk.has_space(aligned)) {
                uint8_t* ptr  = t_chunk.base + t_chunk.used;
                t_chunk.used += aligned;
                return Handle<T>{
                    static_cast<uint32_t>(ptr - m_base), m_base };
            }

            if (!refill_chunk(aligned)) return Handle<T>::null();

            uint8_t* ptr  = t_chunk.base;
            t_chunk.used  = aligned;
            return Handle<T>{
                static_cast<uint32_t>(ptr - m_base), m_base };
        }

        void reset() noexcept {
            m_offset.store(0, std::memory_order_relaxed);
            t_chunk = {};
        }

        [[nodiscard]]
        size_t used() const noexcept {
            return m_offset.load(std::memory_order_relaxed);
        }

        [[nodiscard]]
        size_t capacity() const noexcept { return m_capacity; }
    };

    inline thread_local ScalableArena::ThreadChunk
        ScalableArena::t_chunk{};

} // namespace petronilho::sys