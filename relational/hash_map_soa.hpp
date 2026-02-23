// Layer: L1 | Version: 1.1.0 | Author: Fabio Petronilho de Oliveira
//
// ================================================================
// hash_map_soa.hpp - Tabela Hash Vetorizada Structure of Arrays
// ================================================================
//
// O QUE ESSE CODIGO FAZ:
// Tabela hash com layout SoA onde todas as chaves ficam em um
// array contiguo e todos os valores em outro. Isso permite
// carregar 16 chaves em um unico registrador ZMM e comparar
// todas simultaneamente via find_match_512 do vcore_avx512.
//
// ALGORITMO: Open Addressing com Linear Probing
// BASE TEORICA: Cormen Cap.11.4 - Open Addressing
// Cormen Cap.11.4: sondagem linear h(k,i) = (h(k) + i) mod m
// Cormen Teorema 11.6: com fator de carga a < 1, busca
// mal-sucedida custa O(1/(1-a)) em media.
// Com SoA + AVX512: comparamos 16 slots por ciclo em vez
// de 1, reduzindo custo pratico de O(1) para O(1/16).
//
// APLICACAO EM SEGURANÇA:
// Lookup de IPs suspeitos em tempo real durante ingestao
// de pacotes. Cada pacote recebido consulta a tabela para
// saber se o IP de origem ja esta na lista negra.
// Com AVX512: 16 IPs verificados por ciclo de CPU.
//
// Complexidade: O(1) medio com fator de carga < 0.7
// ================================================================

#pragma once
#include "core/platform/intrinsics_util.hpp"
#include <cstdint>
#include <cstddef>
#include <cstring>

namespace petronilho::relational {

    // Sentinela para slot vazio
    static constexpr uint32_t HASH_EMPTY = 0xFFFFFFFFu;

    struct HashMapSoA {
        uint32_t* m_keys;     // array de chaves alinhado 64B
        uint32_t* m_values;   // array de valores alinhado 64B
        uint32_t  m_capacity; // deve ser potencia de 2

        // ============================================================
        // hash - Funcao de dispersao multiplicativa
        // Cormen Cap.11.3: metodo da multiplicacao
        // Complexidade: O(1)
        // ============================================================
        [[nodiscard]]
        uint32_t hash(uint32_t key) const noexcept {
            // Fibonacci hashing: distribui bem para IPs
            return (key * 2654435769u) & (m_capacity - 1u);
        }

        // ============================================================
        // insert - Insere chave/valor com linear probing
        // Cormen Cap.11.4: INSERT com sondagem linear
        // Complexidade: O(1) amortizado com carga < 0.7
        // ============================================================
        bool insert(uint32_t key, uint32_t value) noexcept {
            uint32_t idx = hash(key);

            for (uint32_t i = 0; i < m_capacity; ++i) {
                const uint32_t slot =
                    (idx + i) & (m_capacity - 1u);

                if (m_keys[slot] == HASH_EMPTY ||
                    m_keys[slot] == key)
                {
                    m_keys[slot]   = key;
                    m_values[slot] = value;
                    return true;
                }
            }
            return false; // tabela cheia
        }

        // ============================================================
        // lookup - Busca chave com SIMD quando disponivel
        // Cormen Cap.11.4: SEARCH com sondagem linear
        // Com AVX512: compara 16 slots por ciclo via ZMM
        // Complexidade: O(1) medio
        // ============================================================
        [[nodiscard]]
        bool lookup(uint32_t  key,
                    uint32_t& out_value) const noexcept
        {
            uint32_t idx = hash(key);

            for (uint32_t i = 0; i < m_capacity; i += 16) {
                const uint32_t slot =
                    (idx + i) & (m_capacity - 1u);

                // Verifica 16 slots simultaneamente via AVX512
                // find_match_512 retorna bitmask de 16 bits
                uint16_t mask = petronilho::platform::
                    find_match_512(&m_keys[slot], key);

                if (mask != 0) {
                    // Encontrou: pega indice do primeiro bit setado
                    uint32_t hit = slot + __builtin_ctz(mask);
                    out_value    = m_values[hit];
                    return true;
                }

                // Verifica se algum slot esta vazio
                // Se sim, a chave nao existe na tabela
                uint16_t empty_mask = petronilho::platform::
                    find_match_512(&m_keys[slot], HASH_EMPTY);
                if (empty_mask != 0) return false;
            }
            return false;
        }

        // Remove chave marcando como HASH_EMPTY
        void remove(uint32_t key) noexcept {
            uint32_t out;
            uint32_t idx = hash(key);

            for (uint32_t i = 0; i < m_capacity; ++i) {
                const uint32_t slot =
                    (idx + i) & (m_capacity - 1u);

                if (m_keys[slot] == HASH_EMPTY) return;
                if (m_keys[slot] == key) {
                    m_keys[slot] = HASH_EMPTY;
                    return;
                }
            }
        }
    };

} // namespace petronilho::relational
