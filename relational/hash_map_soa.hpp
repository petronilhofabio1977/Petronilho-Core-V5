/**
 * @file hash_map_soa.hpp
 * @brief Tabela Hash Vetorizada (Structure of Arrays).
 * * ALGORITMO: Open Addressing / Linear Probing (CLRS Capítulo 11.4).
 * REFERÊNCIA: CLRS Capítulo 11 (Tabelas Hash).
 * * POR QUE: Maximiza o throughput de busca. O layout SoA permite carregar
 * 16 chaves em um único registrador ZMM para comparação paralela.
 */

#pragma once
#include "core/sys/handle.hpp"

namespace super_core::relational {

struct HashMapSoA {
    uint32_t* keys;   // Alinhado a 64B para AVX-512
    uint32_t* values; // Alinhado a 64B para AVX-512
    uint32_t capacity;
};

} // namespace super_core::relational