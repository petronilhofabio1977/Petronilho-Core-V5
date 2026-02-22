// Layer: L0 | Version: 1.1.0 | Author: Fabio Petronilho de Oliveira
//
// ================================================================
// arena_atomic.hpp - Arena Single-Level com Atomic C++23
// ================================================================
//
// O QUE ESSE CODIGO FAZ:
// Arena simples e direta com um unico offset atomico global.
// Mais simples que arena.hpp (sem TLS), mais adequada para
// casos single-thread ou quando simplicidade e prioridade.
//
// DIFERENCA vs arena.hpp:
// arena.hpp    = dois niveis (TLS + global), multi-thread otimizado
// arena_atomic = um nivel (global apenas), mais simples
//
// ALGORITMO: Fetch-and-Add Atomico
// BASE TEORICA: Cormen Cap.17 - Analise Amortizada
//               Cormen Sec.17.1 - Aggregate Analysis
// Cada alocacao e um fetch_add atomico O(1).
// N alocacoes custam O(N) total, O(1) amortizado cada.
//
// CORRECAO vs versao anterior:
// volatile + __sync_fetch_and_add substituidos por
// std::atomic com memory_order correto.
// Motivo: volatile nao garante atomicidade e
// __sync_fetch_and_add nao existe no MSVC (quebra Windows).
// std::atomic e o padrao C++11 portavel em todos compiladores.
// ================================================================

#pragma once
#include "handle.hpp"
#include "core/platform/platform_detect.hpp"
#include <atomic>
#include <cstdint>
#include <cstddef>

namespace petronilho::sys {

    class ArenaAtomic {
    private:
        uint8_t*            m_base;
        size_t              m_capacity;
        std::atomic<size_t> m_offset; // C++11 portavel, substitui volatile

    public:
        ArenaAtomic(void* buffer, size_t size) noexcept
            : m_base(static_cast<uint8_t*>(buffer))
            , m_capacity(size)
            , m_offset(0)
        {}

        ArenaAtomic(const ArenaAtomic&)            = delete;
        ArenaAtomic& operator=(const ArenaAtomic&) = delete;

        // ============================================================
        // allocate<T> - Fetch-and-Add atomico O(1)
        //
        // CORRECAO: memory_order_relaxed e suficiente aqui porque
        // nao ha dependencia de dados entre alocacoes diferentes.
        // Cada thread obtem seu proprio offset unico, sem necessidade
        // de sincronizacao adicional alem do fetch_add em si.
        //
        // Cormen Cap.17: custo amortizado O(1) por alocacao.
        // ============================================================
        template<typename T>
        [[nodiscard]]
        Handle<T> allocate(size_t count = 1) noexcept {
            const size_t raw      = sizeof(T) * count;
            const size_t aligned  = (raw + 15) & ~size_t(15);

            const size_t offset = m_offset.fetch_add(
                aligned, std::memory_order_relaxed);

            if (offset + aligned > m_capacity)
                return Handle<T>::null();

            return Handle<T>{ static_cast<uint32_t>(offset), m_base };
        }

        void reset() noexcept {
            m_offset.store(0, std::memory_order_relaxed);
        }

        [[nodiscard]]
        size_t used() const noexcept {
            return m_offset.load(std::memory_order_relaxed);
        }

        [[nodiscard]]
        size_t capacity() const noexcept { return m_capacity; }
    };

} // namespace petronilho::sys