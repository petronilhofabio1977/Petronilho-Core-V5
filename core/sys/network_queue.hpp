// Layer: L0 | Version: 1.1.0 | Author: Fabio Petronilho de Oliveira
//
// ================================================================
// network_queue.hpp - Fila Lock-Free Single Producer Single Consumer
// ================================================================
//
// O QUE ESSE CODIGO FAZ:
// Fila circular lock-free para transferencia de indices de
// pacotes entre a thread de rede (produtor) e a thread de
// processamento (consumidor).
//
// ALGORITMO: Ring Buffer SPSC Lock-Free
// BASE TEORICA: Cormen Cap.10 - Elementary Data Structures
//               Cormen Sec.10.1 - Stacks and Queues
// Cormen Sec.10.1 descreve filas circulares com head e tail.
// Esta implementacao adiciona atomics para seguranca entre
// duas threads sem lock.
//
// RESTRICAO IMPORTANTE: SPSC apenas
// Single Producer Single Consumer.
// Para multiplos produtores use arena.hpp com CAS.
//
// CORRECAO vs versao anterior:
// - memory_order incorreto causava race condition
// - enqueue: tail com relaxed, head com acquire
// - dequeue: head com relaxed, tail com acquire
// - Capacity deve ser potencia de 2 verificado em compile time
//
// Complexidade: O(1) enqueue e dequeue garantido
// ================================================================

#pragma once
#include <atomic>
#include <cstdint>
#include <cstddef>
#include <cstring>
#include <type_traits>

namespace petronilho::net {

    template<typename T, size_t Capacity>
    class NetworkQueue {
    private:
        // Verifica em compilacao que Capacity e potencia de 2
        static_assert((Capacity & (Capacity - 1)) == 0,
            "Capacity deve ser potencia de 2");
        static_assert(Capacity >= 2,
            "Capacity minimo e 2");

        static constexpr size_t MASK = Capacity - 1;

        // Separa head e tail em cache lines diferentes
        // para evitar False Sharing entre produtor e consumidor
        // Cormen Cap.10: head e tail sao modificados por
        // threads diferentes, separacao evita invalidacao
        // desnecessaria de cache
        alignas(64) std::atomic<size_t> m_head;
        alignas(64) std::atomic<size_t> m_tail;

        T m_buffer[Capacity];

    public:
        NetworkQueue() noexcept : m_head(0), m_tail(0) {
            std::memset(m_buffer, 0, sizeof(m_buffer));
        }

        NetworkQueue(const NetworkQueue&)            = delete;
        NetworkQueue& operator=(const NetworkQueue&) = delete;

        // ============================================================
        // enqueue - Inserir elemento (chamado pelo PRODUTOR)
        //
        // CORRECAO: memory_order correto para SPSC
        // tail: relaxed no load (so o produtor modifica)
        // head: acquire no load (precisa ver escritas do consumidor)
        // tail: release no store (publica novo elemento ao consumidor)
        // ============================================================
        [[nodiscard]]
        bool enqueue(const T& value) noexcept {
            const size_t t = m_tail.load(
                std::memory_order_relaxed);
            const size_t next = (t + 1) & MASK;

            // Fila cheia: next alcancou head
            if (next == m_head.load(std::memory_order_acquire))
                return false;

            m_buffer[t] = value;

            // release: garante que o valor foi escrito
            // antes de atualizar tail
            m_tail.store(next, std::memory_order_release);
            return true;
        }

        // ============================================================
        // dequeue - Remover elemento (chamado pelo CONSUMIDOR)
        //
        // head: relaxed no load (so o consumidor modifica)
        // tail: acquire no load (precisa ver escritas do produtor)
        // head: release no store (publica consumo ao produtor)
        // ============================================================
        [[nodiscard]]
        bool dequeue(T& out) noexcept {
            const size_t h = m_head.load(
                std::memory_order_relaxed);

            // Fila vazia
            if (h == m_tail.load(std::memory_order_acquire))
                return false;

            out = m_buffer[h];

            // release: garante que lemos o valor antes
            // de liberar o slot para o produtor
            m_head.store((h + 1) & MASK,
                std::memory_order_release);
            return true;
        }

        [[nodiscard]]
        bool empty() const noexcept {
            return m_head.load(std::memory_order_acquire)
                == m_tail.load(std::memory_order_acquire);
        }

        [[nodiscard]]
        size_t size() const noexcept {
            const size_t t = m_tail.load(std::memory_order_acquire);
            const size_t h = m_head.load(std::memory_order_acquire);
            return (t - h) & MASK;
        }
    };

} // namespace petronilho::net