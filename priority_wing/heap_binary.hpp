// Layer: L1 | Version: 1.1.0 | Author: Fabio Petronilho de Oliveira
//
// ================================================================
// heap_binary.hpp - Max-Heap Binario sobre Arena
// ================================================================
//
// O QUE ESSE CODIGO FAZ:
// Max-Heap: o maior elemento sempre no topo.
// Complemento do binary_heap.hpp (Min-Heap).
//
// QUANDO USAR CADA UM:
// Min-Heap (binary_heap.hpp): processar alertas do mais
//   urgente para o menos urgente (menor timestamp primeiro)
// Max-Heap (heap_binary.hpp): processar pelo maior valor,
//   ex: pacote com maior anomalia sai primeiro
//
// ALGORITMO: Binary Heap (Max-Heap)
// BASE TEORICA: Cormen Cap.6 - Heapsort
// Cormen Cap.6.2: MAX-HEAPIFY O(log n)
// Cormen Cap.6.5: HEAP-EXTRACT-MAX O(log n)
// Cormen Cap.6.5: MAX-HEAP-INSERT O(log n)
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
    class MaxHeap {
    private:
        petronilho::sys::Handle<T>* m_data;
        size_t                      m_capacity;
        size_t                      m_size;

        [[nodiscard]]
        static size_t parent(size_t i) noexcept { return (i - 1) >> 1; }
        [[nodiscard]]
        static size_t left(size_t i)   noexcept { return (i << 1) + 1; }
        [[nodiscard]]
        static size_t right(size_t i)  noexcept { return (i << 1) + 2; }

        // ============================================================
        // max_heapify - Restaura propriedade do Max-Heap
        // Cormen Cap.6.2: MAX-HEAPIFY iterativo O(log n)
        // CORRECAO: versao anterior era recursiva
        // ============================================================
        void max_heapify(size_t i) noexcept {
            while (true) {
                size_t l       = left(i);
                size_t r       = right(i);
                size_t largest = i;

                if (l < m_size && *m_data[largest] < *m_data[l])
                    largest = l;
                if (r < m_size && *m_data[largest] < *m_data[r])
                    largest = r;

                if (largest == i) break;

                auto tmp         = m_data[i];
                m_data[i]        = m_data[largest];
                m_data[largest]  = tmp;
                i                = largest;
            }
        }

    public:
        MaxHeap(petronilho::sys::ScalableArena& arena,
                size_t max_nodes) noexcept
            : m_capacity(max_nodes)
            , m_size(0)
        {
            auto h = arena.allocate<petronilho::sys::Handle<T>>(max_nodes);
            m_data = h.get_ptr();
        }

        // Cormen Cap.6.5: MAX-HEAP-INSERT O(log n)
        void push(petronilho::sys::Handle<T> handle) noexcept {
            if (m_size >= m_capacity) return;

            size_t i  = m_size++;
            m_data[i] = handle;

            // Sobe enquanto maior que o pai
            while (i > 0 && *m_data[parent(i)] < *m_data[i]) {
                auto tmp          = m_data[i];
                m_data[i]         = m_data[parent(i)];
                m_data[parent(i)] = tmp;
                i                 = parent(i);
            }
        }

        // Cormen Cap.6.5: HEAP-EXTRACT-MAX O(log n)
        [[nodiscard]]
        petronilho::sys::Handle<T> pop() noexcept {
            if (m_size == 0)
                return petronilho::sys::Handle<T>::null();

            auto root  = m_data[0];
            m_data[0]  = m_data[--m_size];
            max_heapify(0);
            return root;
        }

        [[nodiscard]] bool   empty()    const noexcept { return m_size == 0; }
        [[nodiscard]] size_t size()     const noexcept { return m_size; }
        [[nodiscard]] size_t capacity() const noexcept { return m_capacity; }
    };

} // namespace petronilho::priority
