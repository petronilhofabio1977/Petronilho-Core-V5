// Layer: L0 | Version: 1.1.0 | Author: Fabio Petronilho de Oliveira
//
// ================================================================
// handle.hpp - Handle de Offset Relativo ao Pool
// ================================================================
//
// O QUE ESSE CODIGO FAZ:
// Implementa um ponteiro baseado em offset em vez de endereco
// absoluto. Em vez de guardar 0x7fff1234, guarda o indice 42
// relativo ao inicio do pool. Isso permite que o pool seja
// movido na memoria sem invalidar todos os handles.
//
// ALGORITMO: Offset-Based Pointer (Tabela de Indices)
// BASE TEORICA: Cormen Cap.10 - Elementary Data Structures
// Cormen Cap.10.3 descreve implementacao de linked lists
// usando indices de array em vez de ponteiros. Handle<T>
// aplica o mesmo principio: index e relativo ao pool base,
// equivalente ao indice de array do Cormen.
// Complexidade: get_ptr() O(1) - uma soma e um cast
//
// CORRECAO vs versao anterior:
// G_POOL_BASE global removido. Cada Handle agora carrega
// referencia ao seu proprio pool, eliminando acoplamento
// invisivel que causaria bugs silenciosos com multiplos pools.
// ================================================================

#pragma once
#include <cstdint>
#include <cstddef>

namespace petronilho::sys {

    // Sentinel para handle invalido
    static constexpr uint32_t HANDLE_NULL = 0xFFFFFFFFu;

    template<typename T>
    struct Handle {
        uint32_t  m_index;     // offset em bytes do inicio do pool
        uint8_t*  m_pool_base; // ponteiro para o pool dono

        // ============================================================
        // get_ptr - Converte offset em ponteiro real
        // Cormen Cap.10.3: acesso por indice em O(1)
        // ============================================================
        [[nodiscard]]
        T* get_ptr() const noexcept {
            if (m_index == HANDLE_NULL || !m_pool_base)
                return nullptr;
            return reinterpret_cast<T*>(m_pool_base + m_index);
        }

        T& operator*()  const noexcept { return *get_ptr(); }
        T* operator->() const noexcept { return get_ptr();  }

        [[nodiscard]]
        bool is_null() const noexcept {
            return m_index == HANDLE_NULL || !m_pool_base;
        }

        // Comparacao por valor do objeto apontado
        // Cormen Cap.10: ordem relativa entre elementos do pool
        bool operator<(const Handle& other) const noexcept {
            return (*get_ptr()) < (*other.get_ptr());
        }

        // Handle nulo estatico para retorno de erro
        [[nodiscard]]
        static Handle null() noexcept {
            return Handle{ HANDLE_NULL, nullptr };
        }
    };

} // namespace petronilho::sys