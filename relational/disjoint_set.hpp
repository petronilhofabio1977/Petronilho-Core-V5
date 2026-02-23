// Layer: L1 | Version: 1.1.0 | Author: Fabio Petronilho de Oliveira
//
// ================================================================
// disjoint_set.hpp - Union-Find com Path Compression
// ================================================================
//
// O QUE ESSE CODIGO FAZ:
// Agrupa elementos em conjuntos disjuntos e responde em tempo
// quase constante se dois elementos pertencem ao mesmo conjunto.
//
// APLICACAO EM SEGURANÇA:
// Detectar se dois IPs pertencem ao mesmo cluster de ataque.
// Durante um DDoS distribuido, os IPs atacantes formam um
// conjunto. Union-Find identifica em O(alpha(n)) se um novo
// IP ja foi visto no mesmo ataque anterior.
// Tambem detecta ciclos em grafo de conexoes suspeitas.
//
// ALGORITMO: Disjoint-Set Forest
// BASE TEORICA: Cormen Cap.21 - Disjoint Sets
// Cormen Cap.21.3: Union by Rank + Path Compression
// Cormen Teorema 21.14: sequencia de m operacoes em n
// elementos custa O(m * alpha(n)) onde alpha e a funcao
// inversa de Ackermann, na pratica <= 4 para qualquer n
// real. Considerado O(1) amortizado na pratica.
//
// CORRECAO vs versao anterior:
// find_set recursivo substituido por iterativo com
// path compression em dois passos. Recursao causava
// stack overflow em conjuntos com milhoes de elementos.
//
// Complexidade: O(alpha(n)) por operacao, pratica O(1)
// ================================================================

#pragma once
#include <cstdint>
#include <cstddef>

namespace petronilho::relational {

    // ============================================================
    // DSNode - No do Disjoint-Set Forest
    // Cormen Cap.21.3: cada no tem parent e rank
    // ============================================================
    struct DSNode {
        uint32_t m_parent; // indice do representante do conjunto
        uint32_t m_rank;   // limite superior da altura do no
    };

    // ============================================================
    // DisjointSet - Union-Find sobre array externo
    // Array passado externamente para compatibilidade com Arena
    // ============================================================
    class DisjointSet {
    public:

        // ============================================================
        // make_set - Inicializa n conjuntos singleton
        // Cormen Cap.21.3: MAKE-SET, cada elemento e seu proprio pai
        // Complexidade: O(n)
        // ============================================================
        static void make_set(DSNode* arr,
                             size_t  n) noexcept
        {
            for (size_t i = 0; i < n; ++i) {
                arr[i].m_parent = static_cast<uint32_t>(i);
                arr[i].m_rank   = 0;
            }
        }

        // ============================================================
        // find_set - Encontra representante com path compression
        // Cormen Cap.21.3: FIND-SET com path compression dois passos
        // CORRECAO: iterativo em vez de recursivo
        // Passo 1: encontra raiz
        // Passo 2: comprime caminho apontando todos para a raiz
        // Complexidade: O(alpha(n)), pratica O(1)
        // ============================================================
        static uint32_t find_set(DSNode*  arr,
                                 uint32_t i) noexcept
        {
            // Passo 1: encontra raiz iterativamente
            uint32_t root = i;
            while (arr[root].m_parent != root)
                root = arr[root].m_parent;

            // Passo 2: path compression
            // todos os nos no caminho apontam direto para a raiz
            while (arr[i].m_parent != root) {
                uint32_t next    = arr[i].m_parent;
                arr[i].m_parent  = root;
                i                = next;
            }
            return root;
        }

        // ============================================================
        // union_set - Une dois conjuntos por rank
        // Cormen Cap.21.3: UNION by rank
        // Arvore de menor rank fica abaixo da de maior rank
        // Complexidade: O(alpha(n))
        // ============================================================
        static void union_set(DSNode*  arr,
                              uint32_t a,
                              uint32_t b) noexcept
        {
            uint32_t ra = find_set(arr, a);
            uint32_t rb = find_set(arr, b);

            if (ra == rb) return; // ja no mesmo conjunto

            // Cormen Cap.21.3: une por rank
            if (arr[ra].m_rank < arr[rb].m_rank)
                arr[ra].m_parent = rb;
            else if (arr[ra].m_rank > arr[rb].m_rank)
                arr[rb].m_parent = ra;
            else {
                arr[rb].m_parent = ra;
                arr[ra].m_rank++;
            }
        }

        // Verifica se dois elementos pertencem ao mesmo conjunto
        [[nodiscard]]
        static bool same_set(DSNode*  arr,
                             uint32_t a,
                             uint32_t b) noexcept
        {
            return find_set(arr, a) == find_set(arr, b);
        }
    };

} // namespace petronilho::relational
