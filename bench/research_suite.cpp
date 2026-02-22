// Layer: L3 | Version: 2.0.0 | Author: Fabio Petronilho de Oliveira
//
// ================================================================
// Research_suite.cpp - Suite de Benchmark Academico
// ================================================================
//
// O QUE ESSE CODIGO FAZ:
// Mede com precisao de ciclos de CPU o desempenho do Arena
// Allocator do Petronilho Core V5. Usa o contador de hardware
// RDTSC (Read Time-Stamp Counter) para medir latencia real
// sem interferencia do sistema operacional.
//
// ALGORITMOS DO CORMEN UTILIZADOS:
//
// 1. ORDENACAO POR COMPARACAO - Cormen Cap.2 e Cap.7
//    Usado em: std::sort(samples.begin(), samples.end())
//    Por que: Para calcular percentis (P99) precisamos dos
//    dados ordenados. Complexidade O(n log n).
//
// 2. ANALISE AMORTIZADA - Cormen Cap.17
//    Usado em: ScalableArena::allocate()
//    Por que: Cada alocacao custa O(1) amortizado.
//    Cormen Teorema 17.1 (Aggregate Analysis).
//
// 3. BUSCA DE PERCENTIL - Cormen Cap.9 (Order Statistics)
//    Usado em: calculo do P50, P99, P99.9
//    Por que: Apos ordenacao, percentil e acesso O(1).
//
// ================================================================

#include <cpuid.h>
#include <iostream>
#include <vector>
#include <algorithm>
#include <chrono>
#include <numeric>
#include <iomanip>
#include <atomic>
#include <cstring>
#include <x86intrin.h>
#include <sched.h>
#include <thread>
#include <cstdlib>

namespace petronilho::hpc {

    template<typename T>
    inline void do_not_optimize(const T& value) {
        asm volatile("" : : "g"(value) : "memory");
    }

    inline void full_barrier() {
        asm volatile("cpuid" : : : "rax","rbx","rcx","rdx","memory");
    }

    inline uint64_t rdtsc_start() {
        full_barrier();
        return __rdtsc();
    }

    inline uint64_t rdtsc_end() {
        uint32_t aux;
        uint64_t t = __rdtscp(&aux);
        full_barrier();
        return t;
    }

    void set_cpu_affinity(int cpu_id = 0) {
        cpu_set_t cpuset;
        CPU_ZERO(&cpuset);
        CPU_SET(cpu_id, &cpuset);
        if (sched_setaffinity(0, sizeof(cpu_set_t), &cpuset) != 0)
            std::cerr << "Warning: CPU affinity failed.\n";
    }

    bool has_invariant_tsc() {
        uint32_t a, b, c, d;
        unsigned int max_leaf = __get_cpuid_max(0x80000000, nullptr);
        if (max_leaf < 0x80000007) return false;
        if (!__get_cpuid(0x80000007, &a, &b, &c, &d)) return false;
        return (d & (1u << 8)) != 0;
    }

    double estimate_ghz() {
        full_barrier();
        uint64_t t1 = __rdtsc();
        auto c1 = std::chrono::steady_clock::now();
        std::this_thread::sleep_for(std::chrono::milliseconds(200));
        full_barrier();
        uint64_t t2 = __rdtsc();
        auto c2 = std::chrono::steady_clock::now();
        double seconds = std::chrono::duration<double>(c2 - c1).count();
        return (t2 - t1) / seconds / 1e9;
    }

    // ============================================================
    // ScalableArena
    // ALGORITMO: Arena Allocator com Analise Amortizada
    // BASE TEORICA: Cormen Cap.17 - Teorema 17.1
    // Complexidade: O(1) por alocacao, Theta(N) espaco
    // ============================================================
    class ScalableArena {
        uint8_t*            m_memory;
        size_t              m_capacity;
        std::atomic<size_t> m_offset;

    public:
        explicit ScalableArena(size_t size, bool touch_pages = true)
            : m_offset(0)
        {
            const size_t alignment = 4096;
            m_capacity = (size + alignment - 1) & ~(alignment - 1);
            if (posix_memalign(
                    reinterpret_cast<void**>(&m_memory),
                    alignment, m_capacity) != 0)
                std::abort();
            if (touch_pages)
                std::memset(m_memory, 0, m_capacity);
        }

        ~ScalableArena() { std::free(m_memory); }

        [[nodiscard]]
        inline void* allocate(size_t size) noexcept {
            size_t aligned = (size + 15) & ~static_cast<size_t>(15);
            size_t current = m_offset.fetch_add(
                aligned, std::memory_order_relaxed);
            if (current + aligned > m_capacity) return nullptr;
            return m_memory + current;
        }

        void reset() noexcept {
            m_offset.store(0, std::memory_order_relaxed);
        }
    };

    // ============================================================
    // Stats
    // ALGORITMO 1: Ordenacao - Cormen Cap.2 e Cap.7 O(n log n)
    // ALGORITMO 2: Order Statistics - Cormen Cap.9 O(1) pos-sort
    // ============================================================
    struct Stats {
        std::string           m_name;
        std::vector<uint64_t> m_samples;

        void report(double ghz) {
            if (m_samples.empty()) return;

            // Cormen Cap.7: Quicksort O(n log n)
            std::sort(m_samples.begin(), m_samples.end());

            double mean = std::accumulate(
                m_samples.begin(), m_samples.end(), 0.0)
                / m_samples.size();

            // Cormen Cap.9: Selecao O(1) pos-ordenacao
            auto pct = [&](double p) -> uint64_t {
                return m_samples[
                    static_cast<size_t>(p * (m_samples.size()-1))];
            };

            std::cout << std::left << std::setw(32) << m_name
                      << " | Mean: "
                      << std::fixed << std::setprecision(1)
                      << mean / ghz << " ns"
                      << " | P50: "  << pct(0.50)  / ghz << " ns"
                      << " | P99: "  << pct(0.99)  / ghz << " ns"
                      << " | P99.9: "<< pct(0.999) / ghz << " ns"
                      << "\n";
        }
    };

} // namespace petronilho::hpc

int main() {
    using namespace petronilho::hpc;

    set_cpu_affinity(0);

    std::cout << "=== PETRONILHO Academic Research Suite v2.0 ===\n";
    std::cout << "Invariant TSC : "
              << (has_invariant_tsc() ? "Sim" : "Nao") << "\n";

    double ghz = estimate_ghz();
    std::cout << "CPU estimada  : " << ghz << " GHz\n";
    std::cout << "===============================================\n\n";

    {
        const int batches = 10;
        const int iters   = 100000;
        const int warmup  = 50000;

        ScalableArena arena(256ULL * 1024 * 1024);

        for (int i = 0; i < warmup; ++i) {
            void* p = arena.allocate(64);
            do_not_optimize(p);
        }
        arena.reset();

        Stats stats;
        stats.m_name = "Arena::allocate(64B)";
        stats.m_samples.reserve(batches * iters);

        for (int b = 0; b < batches; ++b) {
            arena.reset();
            for (int i = 0; i < iters; ++i) {
                uint64_t t = rdtsc_start();
                void*    p = arena.allocate(64);
                uint64_t e = rdtsc_end();
                if (!p) std::abort();
                do_not_optimize(p);
                stats.m_samples.push_back(e - t);
            }
        }

        stats.report(ghz);
    }

    std::cout << "\n[CONCLUIDO] Suite finalizada.\n";
    return 0;
}