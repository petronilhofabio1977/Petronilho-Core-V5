// Layer: L1 | Version: 1.1.0 | Author: Fabio Petronilho de Oliveira
//
// ================================================================
// binary_heap.hpp - Min-Heap Binario sobre Arena
// ================================================================
//
// O QUE ESSE CODIGO FAZ:
// Implementa fila de prioridade minima sobre memoria da Arena.
// Usado no scheduler para processar eventos/pacotes em ordem
// de prioridade sem alocacao dinamica.
//
// ALGORITMO: Binary Heap (Min-Heap)
// BASE TEORICA: Cormen Cap.6 - Heapsort
// Cormen Cap.6.1: heap e arvore binaria quase completa
// representada como array. Pai de i = (i-1)/2, filho
// esquerdo = 2i+1, filho direito = 2i+2.
// Cormen Cap.6.2: MAX-HEAPIFY em O(log n)
// Cormen Cap.6.5: operacoes de fila de prioridade
//   push (INSERT): O(log n)
//   pop  (EXTRACT-MIN): O(log n)
//
// APLICACAO EM SEGURANÇA:
// Processar alertas de anomalia em ordem de severidade.
// O pacote mais anomalo (maior desvio do P99) sai primeiro.
//
// Complexidade Tempo : push O(log n), pop O(log n)
// Complexidade Espaco: O(n) na Arena, zero malloc
// ================================================================

#pragma once
#include "core/sys/arena.hpp"
#include "core/sys/handle.hpp"
#include <cstddef>

namespace petronilho::priority {

    template<typename T>
    class BinaryHeap {
    private:
        petronilho::sys::Handle<T>* m_data;
        size_t                      m_capacity;
        size_t                      m_size;

        // Cormen Cap.6.1: relacoes pai/filho em array
        [[nodiscard]]
        static size_t parent(size_t i) noexcept { return (i - 1) >> 1; }
        [[nodiscard]]
        static size_t left(size_t i)   noexcept { return (i << 1) + 1; }
        [[nodiscard]]
        static size_t right(size_t i)  noexcept { return (i << 1) + 2; }

        // ============================================================
        // min_heapify - Restaura propriedade do heap
        // BASE TEORICA: Cormen Cap.6.2 - MAX-HEAPIFY adaptado para MIN
        // CORRECAO: versao anterior era recursiva, risco de stack
        // overflow em heaps grandes. Substituido por loop iterativo.
        // Complexidade: O(log n)
        // ============================================================
        void min_heapify(size_t i) noexcept {
            while (true) {
                size_t l        = left(i);
                size_t r        = right(i);
                size_t smallest = i;

                if (l < m_size && *m_data[l] < *m_data[smallest])
                    smallest = l;
                if (r < m_size && *m_data[r] < *m_data[smallest])
                    smallest = r;

                if (smallest == i) break;

                // Cormen Cap.6.2: troca e desce
                auto tmp        = m_data[i];
                m_data[i]       = m_data[smallest];
                m_data[smallest]= tmp;
                i               = smallest;
            }
        }

    public:
        BinaryHeap(petronilho::sys::ScalableArena& arena,
                   size_t max_nodes) noexcept
            : m_capacity(max_nodes)
            , m_size(0)
        {
            auto h   = arena.allocate<petronilho::sys::Handle<T>>(max_nodes);
            m_data   = h.get_ptr();
        }

        // ============================================================
        // push - Insere elemento mantendo propriedade do heap
        // Cormen Cap.6.5 HEAP-INCREASE-KEY adaptado para min-heap
        // Complexidade: O(log n)
        // ============================================================
        void push(petronilho::sys::Handle<T> handle) noexcept {
            if (m_size >= m_capacity) return;

            size_t i   = m_size++;
            m_data[i]  = handle;

            // Sobe enquanto menor que o pai
            while (i > 0 && *m_data[i] < *m_data[parent(i)]) {
                auto tmp          = m_data[i];
                m_data[i]         = m_data[parent(i)];
                m_data[parent(i)] = tmp;
                i                 = parent(i);
            }
        }

        // ============================================================
        // pop - Remove e retorna o menor elemento
        // Cormen Cap.6.5 EXTRACT-MIN
        // Complexidade: O(log n)
        // CORRECAO: retorna Handle::null() em vez de 0xFFFFFFFF
        // ============================================================
        [[nodiscard]]
        petronilho::sys::Handle<T> pop() noexcept {
            if (m_size == 0)
                return petronilho::sys::Handle<T>::null();

            auto root  = m_data[0];
            m_data[0]  = m_data[--m_size];
            min_heapify(0);
            return root;
        }

        [[nodiscard]]
        bool empty() const noexcept { return m_size == 0; }

        [[nodiscard]]
        size_t size() const noexcept { return m_size; }

        [[nodiscard]]
        size_t capacity() const noexcept { return m_capacity; }
    };

} // namespace petronilho::priority
