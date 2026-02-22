// Layer: L0 | Version: 1.1.0 | Author: Fabio Petronilho de Oliveira
//
// ================================================================
// arena.hpp - Arena Alocador com Thread-Local Storage
// ================================================================
//
// O QUE ESSE CODIGO FAZ:
// Arena de dois niveis: cada thread tem um chunk local de 64KB
// (alocacao sem lock, zero CAS). Quando o chunk local esgota,
// busca novo bloco no pool global via CAS atomico.
// Resultado: alocacao O(1) sem contencao na grande maioria
// das operacoes.
//
// ALGORITMO: Two-Level Arena com CAS
// BASE TEORICA: Cormen Cap.17 - Analise Amortizada
//               Cormen Cap.26 - Maximum Flow (CAS como arbitro)
// Nivel 1 (Local): alocacao por incremento de ponteiro O(1)
//                  zero contencao, zero CAS, zero lock
// Nivel 2 (Global): CAS atomico apenas quando chunk local
//                   esgota. Frequencia: 1 CAS a cada 64KB
//                   alocados. Custo amortizado O(1).
//
// DIFERENCA vs ScalableArena do Research_suite:
// Research_suite: Arena single-thread simples, sem chunks
// Este Arena: Multi-thread com TLS, producao real
// ================================================================

#pragma once
#include "handle.hpp"
#include "core/platform/platform_detect.hpp"
#include <atomic>
#include <cstdint>
#include <cstddef>

namespace petronilho::sys {

    // ============================================================
    // LocalChunk - Estado por thread (Thread-Local Storage)
    // Cormen Cap.17: cada thread mantem seu proprio contador
    // de offset local, eliminando contencao no caso comum.
    // ============================================================
    struct alignas(64) LocalChunk {
        uint32_t m_current; // proximo offset disponivel no chunk
        uint32_t m_end;     // fim do chunk atual

        [[nodiscard]]
        bool has_space(size_t needed) const noexcept {
            return m_current + needed <= m_end;
        }
    };

    // ============================================================
    // ScalableArena - Arena de dois niveis thread-safe
    //
    // ALGORITMO: Two-Level TLS Arena
    // BASE TEORICA: Cormen Cap.17 Teorema 17.1
    // Complexidade amortizada: O(1) por alocacao
    // Thread-safety: sim, via TLS + CAS global
    // Excecoes: nenhuma
    //
    // Invariante: buffer externo deve existir durante
    // todo o ciclo de vida da Arena
    // ============================================================
    class ScalableArena {
    private:
        uint8_t*             m_base;
        size_t               m_capacity;
        std::atomic<size_t>  m_global_offset;

        // 64KB por chunk: equilibrio entre contencao global
        // e desperdicio de memoria por thread
        static constexpr size_t CHUNK_SIZE = 64 * 1024;

        // TLS: cada thread tem seu proprio chunk local
        // zero contencao no caso comum
        static thread_local LocalChunk t_chunk;

        // ============================================================
        // refill_chunk - Busca novo chunk no pool global via CAS
        // Cormen Cap.17: operacao cara executada raramente,
        // amortizando o custo sobre N alocacoes subsequentes.
        // Complexidade: O(1) amortizado, O(k) pior caso com
        // k threads competindo pelo mesmo slot.
        // ============================================================
        [[nodiscard]]
        bool refill_chunk() noexcept {
            size_t current, next;
            do {
                current = m_global_offset.load(
                    std::memory_order_relaxed);
                next = current + CHUNK_SIZE;
                if (next > m_capacity) return false;
            } while (!m_global_offset.compare_exchange_weak(
                current, next,
                std::memory_order_acq_rel,
                std::memory_order_relaxed));

            t_chunk.m_current = static_cast<uint32_t>(current);
            t_chunk.m_end     = static_cast<uint32_t>(next);
            return true;
        }

    public:
        ScalableArena(void* buffer, size_t size) noexcept
            : m_base(static_cast<uint8_t*>(buffer))
            , m_capacity(size)
            , m_global_offset(0)
        {}

        // Nao copiavel, nao movivel
        ScalableArena(const ScalableArena&)            = delete;
        ScalableArena& operator=(const ScalableArena&) = delete;

        // ============================================================
        // allocate<T> - Alocacao O(1) amortizado
        // Caso comum: incremento local sem lock, sem CAS
        // Caso raro: CAS global para novo chunk (1 a cada 64KB)
        // ============================================================
        template<typename T>
        [[nodiscard]]
        Handle<T> allocate() noexcept {
            const size_t size_needed =
                (sizeof(T) + 15) & ~size_t(15); // alinha 16 bytes

            // Caso comum: chunk local tem espaco
            if (!t_chunk.has_space(size_needed)) {
                // Caso raro: busca novo chunk no global
                if (!refill_chunk())
                    return Handle<T>::null();
            }

            const uint32_t index = t_chunk.m_current;
            t_chunk.m_current   += static_cast<uint32_t>(size_needed);

            return Handle<T>{ index, m_base };
        }

        // Reset global: apenas para uso single-thread
        void reset() noexcept {
            m_global_offset.store(0, std::memory_order_relaxed);
            t_chunk = {0, 0};
        }

        [[nodiscard]]
        size_t used() const noexcept {
            return m_global_offset.load(std::memory_order_relaxed);
        }

        [[nodiscard]]
        size_t capacity() const noexcept { return m_capacity; }
    };

    // Definicao do TLS fora da classe (necessario em C++)
    inline thread_local LocalChunk ScalableArena::t_chunk = {0, 0};

} // namespace petronilho::sys