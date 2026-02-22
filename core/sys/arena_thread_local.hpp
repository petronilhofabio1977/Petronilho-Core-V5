// Layer: L0 | Version: 1.1.0 | Author: Fabio Petronilho de Oliveira
//
// ================================================================
// arena_thread_local.hpp - Arena Exclusiva por Thread
// ================================================================
//
// O QUE ESSE CODIGO FAZ:
// Cada thread tem sua propria arena independente de 64MB.
// Alocacoes sao O(1) sem nenhum lock ou CAS porque nenhuma
// outra thread compartilha o mesmo bloco de memoria.
//
// DIFERENCA vs arena.hpp e arena_atomic.hpp:
// arena.hpp        = TLS + pool global compartilhado
// arena_atomic.hpp = pool global com CAS
// este arquivo     = pool completamente isolado por thread
//                    zero sincronizacao, zero contencao
//
// ALGORITMO: Arena Monotonica por Thread
// BASE TEORICA: Cormen Cap.17 - Analise Amortizada
// Complexidade: O(1) garantido, sem CAS, sem lock
// Thread-safety: total por isolamento, nao por sincronizacao
//
// CORRECOES vs versao anterior:
// - posix_memalign substituido por aligned_alloc_portable
// - new/delete substituidos por alocacao direta (proibidos L0)
// - Overflow de bloco retorna nullptr em vez de criar novo bloco
//   silenciosamente e perder dados
// ================================================================

#pragma once
#include "core/platform/memory_util.hpp"
#include <cstddef>
#include <cstdint>
#include <cstdlib>

namespace petronilho::sys {

    class ThreadLocalArena {
    private:

        struct Block {
            uint8_t* m_memory;
            size_t   m_capacity;
            size_t   m_offset;

            explicit Block(size_t size) noexcept
                : m_capacity(size), m_offset(0)
            {
                // CORRECAO: usa aligned_alloc_portable
                // em vez de posix_memalign (Linux only)
                m_memory = static_cast<uint8_t*>(
                    petronilho::platform::aligned_alloc_portable(
                        4096, size));
            }

            ~Block() noexcept {
                petronilho::platform::aligned_free_portable(m_memory);
            }

            [[nodiscard]]
            void* allocate(size_t size) noexcept {
                const size_t aligned =
                    (size + 15) & ~size_t(15);

                if (!m_memory || m_offset + aligned > m_capacity)
                    return nullptr;

                void* ptr  = m_memory + m_offset;
                m_offset  += aligned;
                return ptr;
            }

            void reset() noexcept { m_offset = 0; }

            [[nodiscard]]
            bool valid() const noexcept {
                return m_memory != nullptr;
            }
        };

        static constexpr size_t DEFAULT_SIZE = 64ULL * 1024 * 1024;

        static thread_local Block* t_block;

    public:

        // initialize deve ser chamado uma vez por thread
        // antes de qualquer alocacao
        static void initialize(
            size_t size = DEFAULT_SIZE) noexcept
        {
            if (t_block) return;

            // CORRECAO: usa alocacao direta em vez de new
            void* mem = petronilho::platform::aligned_alloc_portable(
                alignof(Block), sizeof(Block));
            if (!mem) return;

            t_block = new (mem) Block(size);

            if (!t_block->valid()) {
                t_block->~Block();
                petronilho::platform::aligned_free_portable(mem);
                t_block = nullptr;
            }
        }

        [[nodiscard]]
        static void* allocate(size_t size) noexcept {
            if (!t_block) return nullptr;

            // CORRECAO: retorna nullptr se cheio
            // Versao anterior criava novo bloco silenciosamente
            // e perdia referencia ao bloco anterior (memory leak)
            return t_block->allocate(size);
        }

        static void reset() noexcept {
            if (t_block) t_block->reset();
        }

        static void shutdown() noexcept {
            if (!t_block) return;
            t_block->~Block();
            petronilho::platform::aligned_free_portable(t_block);
            t_block = nullptr;
        }

        [[nodiscard]]
        static size_t used() noexcept {
            return t_block ? t_block->m_offset : 0;
        }

        [[nodiscard]]
        static bool ready() noexcept {
            return t_block && t_block->valid();
        }
    };

    inline thread_local ThreadLocalArena::Block*
        ThreadLocalArena::t_block = nullptr;

} // namespace petronilho::sys