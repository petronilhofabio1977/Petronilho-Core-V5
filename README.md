# SUPER-CORE Runtime v1.0

**Projeto:** Motor de execução de alta performance e baixa latência, otimizado para Intel Westmere.

## Estrutura
- core/: Alocadores, handles, dispatch de CPU
- geometric_wing/: Detecção de colisão AABB, árvores espaciais
- priority_wing/: Heaps e schedulers
- relational/: Grafos, hash tables, algoritmos CLRS
- perf/: Testes de stress, benchmarks e telemetria
- Governance/: Documentação, dependências, protocolos

## Build
```bash
mkdir build && cd build
cmake ..
make
```

## Testes
- stress_test_1m → Stress massivo de colisões
- test_concurrency → Benchmark lock-free multithread
- test_arena → Alocador ScalableArena
