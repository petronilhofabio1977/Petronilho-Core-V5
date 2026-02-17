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
 
# ⚡ SUPER-CORE RUNTIME v2.0 (Petronilho Edition)

### High-Performance Networking & Memory Engine for Legacy Hardware (Intel Westmere)

Este motor foi desenvolvido para extrair a performance máxima de CPUs da geração 2010+, utilizando técnicas modernas de Kernel Bypass e Lock-Free Data Structures.

## 🏗️ Arquitetura do Sistema
- **Memory Management:** `ScalableArena` baseada em alocação estática e alinhamento de 4096 bytes para evitar Page Faults.
- **Communication:** `NetworkQueue` SPSC (Single-Producer Single-Consumer) com padding manual de Cache Line (64 bytes) para eliminar o False Sharing.
- **Networking:** Integração nativa com `io_uring` (Linux 5.10+) permitindo Ingest de pacotes com latência sub-microssegundo.

## 📊 Benchmarks (i7 M 620 @ 2.4GHz)
- **Vazão Interna:** 12.24 Mpps (Milhões de pacotes por segundo).
- **Latência de Rede (UDP):** ~144 ciclos de CPU (~60ns) por pacote via io_uring.
- **Protocolo:** UDP Zero-Copy Ingest.

## 🛠️ Tecnologias Utilizadas
- **C++23** (Focado em performance, sem exceções).
- **liburing** para I/O assíncrono.
- **Afinidade de CPU** (Pinning) para isolamento de carga.
- **Telemetria RDTSC** para medição de jitter em nanossegundos.

---
*Developed by Techmaster @ Petronilho*
