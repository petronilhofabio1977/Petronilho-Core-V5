/**
 * @file heap_binary.hpp
 * @brief Heap Binário Máximo para Filas de Prioridade.
 * * ALGORITMO: Heapsort / Max-Heapify (CLRS Capítulo 6).
 * REFERÊNCIA: CLRS Capítulo 6 (Heapsort).
 * * POR QUE: Fornece a base para o escalonamento de tarefas. O layout em array 
 * permite que o pai de um nó no índice i seja encontrado em (i-1)/2 via shift.
 */

#pragma once
#include "core/sys/handle.hpp"
#include "core/sys/arena.hpp"

namespace super_core::priority {

template <typename T>
class BinaryHeap {
    sys::Handle<T> data_base;
    uint32_t current_size;
    uint32_t max_capacity;

public:
    BinaryHeap(sys::Handle<T> base, uint32_t capacity) 
        : data_base(base), current_size(0), max_capacity(capacity) {}

    /**
     * @brief Mantém a propriedade de Max-Heap.
     * Complexidade: O(log N)
     * Impacto: Localidade de cache decrescente conforme desce na árvore.
     */
    void max_heapify(uint32_t i, sys::Arena& arena) noexcept {
        T* arr = arena.resolve(data_base);
        uint32_t largest = i;
        uint32_t left = 2 * i + 1;
        uint32_t right = 2 * i + 2;

        if (left < current_size && arr[left] > arr[largest]) largest = left;
        if (right < current_size && arr[right] > arr[largest]) largest = right;

        if (largest != i) {
            T temp = arr[i];
            arr[i] = arr[largest];
            arr[largest] = temp;
            max_heapify(largest, arena);
        }
    }

    // Invariante: O elemento no índice 0 é sempre o maior (prioridade máxima).
};

} // namespace super_core::priority