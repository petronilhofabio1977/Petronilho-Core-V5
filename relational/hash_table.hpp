// Layer: L1 | Version: 1.1.0 | Author: Fabio Petronilho de Oliveira
//
// ================================================================
// hash_table.hpp - Tabela Hash Generica com Handle
// ================================================================
//
// O QUE ESSE CODIGO FAZ:
// Tabela hash generica que armazena Handle<V> em vez do valor
// direto. Complementa hash_map_soa.hpp que e especializado
// para uint32_t com SIMD.
//
// QUANDO USAR CADA UM:
// hash_map_soa.hpp : lookup de IPs uint32_t com AVX512
// hash_table.hpp   : lookup de tipos arbitrarios com Handle
//
// ALGORITMO: Open Addressing com Linear Probing
// BASE TEORICA: Cormen Cap.11.4 - Open Addressing
// Cormen Cap.11.2: hash por divisao h(k) = k mod m
// Cormen Cap.11.4: sondagem linear para colisoes
// Complexidade: O(1) amortizado com carga < 0.7
// ================================================================

#pragma once
#include "core/sys/arena.hpp"
#include "core/sys/handle.hpp"
#include <cstddef>
#include <cstring>

namespace petronilho::relational {

    template<typename K, typename V>
    struct HashEntry {
        K                          m_key;
        petronilho::sys::Handle<V> m_handle;
        bool                       m_occupied;
    };

    template<typename K, typename V>
    class HashTable {
    private:
        HashEntry<K, V>* m_table;
        size_t           m_capacity;

        // ============================================================
        // hash - Funcao de dispersao
        // Cormen Cap.11.3: metodo da divisao h(k) = k mod m
        // CORRECAO: usa bitmask em vez de % para potencia de 2
        // % e divisao inteira, lento. Bitmask e O(1) rapido.
        // ============================================================
        [[nodiscard]]
        size_t hash(K key) const noexcept {
            return static_cast<size_t>(key)
                & (m_capacity - 1u);
        }

    public:
        HashTable(petronilho::sys::ScalableArena& arena,
                  size_t max_entries) noexcept
            : m_capacity(max_entries)
        {
            auto h   = arena.allocate<HashEntry<K,V>>(max_entries);
            m_table  = h.get_ptr();

            // Inicializa todos os slots como vazios
            for (size_t i = 0; i < m_capacity; ++i)
                m_table[i].m_occupied = false;
        }

        // ============================================================
        // insert - Insere com linear probing
        // Cormen Cap.11.4: INSERT O(1) amortizado
        // ============================================================
        void insert(K key,
                    petronilho::sys::Handle<V> handle) noexcept
        {
            size_t h = hash(key);

            for (size_t i = 0; i < m_capacity; ++i) {
                const size_t slot = (h + i) & (m_capacity - 1u);

                if (!m_table[slot].m_occupied ||
                     m_table[slot].m_key == key)
                {
                    m_table[slot].m_key      = key;
                    m_table[slot].m_handle   = handle;
                    m_table[slot].m_occupied = true;
                    return;
                }
            }
        }

        // ============================================================
        // get - Busca com linear probing
        // Cormen Cap.11.4: SEARCH O(1) amortizado
        // CORRECAO: retorna Handle::null() em vez de 0xFFFFFFFF
        // ============================================================
        [[nodiscard]]
        petronilho::sys::Handle<V> get(K key) const noexcept {
            size_t h     = hash(key);
            size_t start = h;

            for (size_t i = 0; i < m_capacity; ++i) {
                const size_t slot = (h + i) & (m_capacity - 1u);

                if (!m_table[slot].m_occupied)
                    return petronilho::sys::Handle<V>::null();

                if (m_table[slot].m_key == key)
                    return m_table[slot].m_handle;
            }
            return petronilho::sys::Handle<V>::null();
        }

        [[nodiscard]]
        bool contains(K key) const noexcept {
            petronilho::sys::Handle<V> h = get(key);
            return !h.is_null();
        }
    };

} // namespace petronilho::relational
