#include "arena.hpp"
#include <iostream>
#include <vector>
#include <thread>
#include <chrono>
#include <algorithm>
#include <iomanip>
#include <numeric>

using namespace petronilho;
using namespace std::chrono;

struct LatencyReport {
    double mean;
    uint64_t p95, p99, p999, p9999, max;
};

LatencyReport calculate_stats(std::vector<uint64_t>& samples) {
    std::sort(samples.begin(), samples.end());
    uint64_t sum = std::accumulate(samples.begin(), samples.end(), 0ULL);
    return {
        (double)sum / samples.size(),
        samples[samples.size() * 0.95],
        samples[samples.size() * 0.99],
        samples[samples.size() * 0.999],
        samples[samples.size() * 0.9999],
        samples.back()
    };
}

void stress_test() {
    const int num_threads = std::thread::hardware_concurrency();
    const size_t arena_size = 1ULL * 1024 * 1024 * 1024; // 1GB para caber na RAM da VM
    ScalableArena arena(arena_size);
    
    std::vector<std::vector<uint64_t>> all_samples(num_threads);
    std::vector<std::thread> threads;

    std::cout << "--- INICIANDO TESTE DE ESTRESSE DE PRODUÇÃO (TIER 1) ---" << std::endl;
    std::cout << "Threads Detectadas: " << num_threads << " | Carga: 10M alocações/thread" << std::endl;

    auto t_start_global = high_resolution_clock::now();

    for (int t = 0; t < num_threads; ++t) {
        threads.emplace_back([&arena, &all_samples, t]() {
            all_samples[t].reserve(10'000'000);
            for (int i = 0; i < 10'000'000; ++i) {
                auto t1 = high_resolution_clock::now();
                void* p = arena.allocate(64);
                auto t2 = high_resolution_clock::now();
                
                if (!p) {
                    arena.reset(); 
                    p = arena.allocate(64);
                }
                
                all_samples[t].push_back(duration_cast<nanoseconds>(t2 - t1).count());
                asm volatile("" : : "g"(p) : "memory");
            }
        });
    }

    for (auto& t : threads) t.join();
    auto t_end_global = high_resolution_clock::now();

    // Consolidar e calcular estatísticas
    std::vector<uint64_t> global_samples;
    global_samples.reserve(num_threads * 10'000'000);
    for (const auto& s : all_samples) {
        global_samples.insert(global_samples.end(), s.begin(), s.end());
    }

    auto report = calculate_stats(global_samples);
    double total_sec = duration_cast<milliseconds>(t_end_global - t_start_global).count() / 1000.0;

    std::cout << "\n--- RESULTADOS MICROARQUITETURAIS ---" << std::endl;
    std::cout << "Tempo Total: " << total_sec << " s" << std::endl;
    std::cout << "Média:       " << std::fixed << std::setprecision(2) << report.mean << " ns" << std::endl;
    std::cout << "P99:         " << report.p99 << " ns" << std::endl;
    std::cout << "P99.99:      " << report.p9999 << " ns (Tail Latency)" << std::endl;
    std::cout << "Max:         " << report.max << " ns" << std::endl;
    
    std::cout << "\n--- AUDITORIA DE ESTABILIDADE ---" << std::endl;
    if (report.p9999 < report.mean * 20) {
        std::cout << "STATUS: ESTÁVEL (Baixa variância de cauda)" << std::endl;
    } else {
        std::cout << "STATUS: RUÍDO DETECTADO (SO interrompeu o kernel)" << std::endl;
    }
}

int main() {
    stress_test();
    return 0;
}
