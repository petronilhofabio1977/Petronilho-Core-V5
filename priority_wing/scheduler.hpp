/**
 * @file scheduler.hpp
 * @brief Escalonador de Tarefas Determinístico (Work-Stealing Ready).
 * * ALGORITMO: Priority Queue Dispatching.
 * REFERÊNCIA: CLRS Capítulo 6.5 (Filas de Prioridade).
 * * POR QUE: Minimiza o 'idle time' dos cores AVX-512 garantindo que a 
 * tarefa de maior impacto computacional seja despachada imediatamente.
 */

#pragma once
#include "core/sys/handle.hpp"
#include "core/sys/arena.hpp"
#include "priority_wing/heap_binary.hpp"

namespace super_core::priority {

struct Task {
    uint32_t priority;
    void (*entry_point)(void*); // Ponteiro para Kernel (vcore_avxXXX)
    void* args;

    // Operador de comparação para o BinaryHeap
    bool operator>(const Task& other) const noexcept {
        return priority > other.priority;
    }
};

class TaskScheduler {
    BinaryHeap<Task> queue;

public:
    explicit TaskScheduler(sys::Handle<Task> storage, uint32_t capacity)
        : queue(storage, capacity) {}

    /**
     * @brief Insere uma nova tarefa no runtime.
     * Complexidade: O(log N)
     */
    void schedule(const Task& t, sys::Arena& arena) noexcept {
        // Lógica de inserção e 'bubble-up' no heap
    }

    /**
     * @brief Despacha a tarefa de maior prioridade para o CPU.
     * Complexidade: O(log N) para re-heapify
     */
    void dispatch_next(sys::Arena& arena) noexcept {
        // Extrai raiz do heap e executa entry_point
    }
};

} // namespace super_core::priority