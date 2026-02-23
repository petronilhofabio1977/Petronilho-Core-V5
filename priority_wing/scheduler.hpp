// Layer: L1 | Version: 1.1.0 | Author: Fabio Petronilho de Oliveira
//
// ================================================================
// scheduler.hpp - Escalonador de Tarefas por Prioridade
// ================================================================
//
// O QUE ESSE CODIGO FAZ:
// Escalonador determinístico usando Max-Heap. A tarefa de
// maior prioridade é sempre despachada primeiro.
//
// APLICACAO EM SEGURANÇA:
// Alertas de anomalia critica (prioridade 100) sao processados
// antes de alertas informativos (prioridade 1).
// Garante que um ataque DDoS em andamento nao seja bloqueado
// por tarefas de baixa prioridade na fila.
//
// ALGORITMO: Priority Queue Dispatching
// BASE TEORICA: Cormen Cap.6.5 - Priority Queues
// Cormen Cap.6.5: MAX-HEAP-INSERT O(log n)
// Cormen Cap.6.5: HEAP-EXTRACT-MAX O(log n)
// Resultado: tarefa mais critica despachada em O(log n)
//
// Complexidade Tempo : schedule O(log n), dispatch O(log n)
// Complexidade Espaco: O(n) na Arena
// ================================================================

#pragma once
#include "core/sys/handle.hpp"
#include "core/sys/arena.hpp"
#include "priority_wing/heap_binary.hpp"
#include <cstdint>

namespace petronilho::priority {

    // ============================================================
    // Task - Unidade de trabalho escalonavel
    // priority: maior valor = maior urgencia
    // entry_point: funcao a executar (vcore_avx2, vcore_sse4, etc)
    // args: argumentos para a funcao
    // ============================================================
    struct Task {
        uint32_t  m_priority;
        void    (*m_entry_point)(void*);
        void*     m_args;

        // Cormen Cap.6: comparacao para Max-Heap
        // maior prioridade = maior elemento no heap
        bool operator<(const Task& other) const noexcept {
            return m_priority < other.m_priority;
        }
    };

    // ============================================================
    // TaskScheduler - Despacha tarefas em ordem de prioridade
    // ============================================================
    class TaskScheduler {
    private:
        MaxHeap<Task> m_queue;

    public:
        explicit TaskScheduler(
            petronilho::sys::ScalableArena& arena,
            size_t capacity) noexcept
            : m_queue(arena, capacity)
        {}

        // ============================================================
        // schedule - Insere tarefa na fila de prioridade
        // Cormen Cap.6.5: MAX-HEAP-INSERT O(log n)
        // ============================================================
        void schedule(petronilho::sys::Handle<Task> handle) noexcept {
            m_queue.push(handle);
        }

        // ============================================================
        // dispatch_next - Executa tarefa de maior prioridade
        // Cormen Cap.6.5: HEAP-EXTRACT-MAX O(log n)
        // Retorna false se fila vazia
        // ============================================================
        bool dispatch_next() noexcept {
            if (m_queue.empty()) return false;

            auto handle = m_queue.pop();
            if (handle.is_null()) return false;

            Task* task = handle.get_ptr();
            if (!task || !task->m_entry_point) return false;

            // Executa o kernel: vcore_sse4, vcore_avx2 ou vcore_avx512
            task->m_entry_point(task->m_args);
            return true;
        }

        [[nodiscard]]
        bool empty() const noexcept { return m_queue.empty(); }

        [[nodiscard]]
        size_t size() const noexcept { return m_queue.size(); }
    };

} // namespace petronilho::priority
