# Petronilho Core V5

Sistema de ingestão UDP de baixa latência em C++17, desenvolvido por Fábio Petronilho de Oliveira.

## Resultados Reais

Testado em Intel Core i7-620M (2010) com Arch Linux:

| Métrica | Resultado |
|--------|-----------|
| P50 de latência | 2,7 microssegundos |
| P99 de latência | 27,7 microssegundos |
| Pacotes em 300 segundos | 48,2 milhões |
| Hardware | i7-620M, 2010 |

## Técnicas Implementadas

- Arena Allocator monotônico com complexidade O(1)
- Ring Buffer lock-free com mmap zero-copy
- CPU Affinity via sched_setaffinity
- Socket UDP não bloqueante
- Timestamping de alta resolução

## Base Teórica

Algoritmos baseados em Cormen et al. (CLRS): Capítulo 10 (Filas), Capítulo 11 (Gerenciamento de Memória), Capítulo 17 (Análise Amortizada).

## Status

Protótipo funcional em desenvolvimento ativo. Protocol Decoder e Shared Memory API em implementação.

## Licença

MIT License

