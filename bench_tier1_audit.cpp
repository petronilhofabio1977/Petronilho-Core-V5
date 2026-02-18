#include "arena.hpp"
#include <iostream>
#include <vector>
#include <thread>
#include <chrono>
#include <algorithm>
#include <iomanip>
#include <map>
#include <mutex>

using namespace petronilho;
using namespace std::chrono;

void run_audit_session() {
    const int num_threads = std::thread::hardware_concurrency();
    const size_t ops_per_thread = 5'000'000;
    ScalableArena arena(1ULL * 1024 * 1024 * 1024);

    struct Bucket {
        uint64_t count = 0;
        std::string label;
    };
    
    std::map<uint64_t, Bucket> histogram = {
        {100, {0, "< 100ns "}},
        {250, {0, "< 250ns "}},
        {500, {0, "< 500ns "}},
        {1000, {0, "< 1us   "}},
        {5000, {0, "< 5us   "}},
        {10000, {0, "< 10us  "}},
        {100000, {0, "< 100us "}},
        {1000000, {0, "< 1ms   "}},
        {UINT64_MAX, {0, "Outliers"}}
    };

    std::vector<std::thread> threads;
    std::vector<std::vector<uint64_t>> results(num_threads);

    std::cout << "--- PETRONILHO TIER-1 LATENCY AUDIT ---" << std::endl;
    std::cout << "Monitorando " << num_threads * ops_per_thread << " operações..." << std::endl;

    for (int t = 0; t < num_threads; ++t) {
        threads.emplace_back([&arena, &results, t, ops_per_thread]() {
            results[t].reserve(ops_per_thread);
            for (size_t i = 0; i < ops_per_thread; ++i) {
                auto t1 = high_resolution_clock::now();
                void* p = arena.allocate(64);
                auto t2 = high_resolution_clock::now();

                if (!p) { arena.reset(); p = arena.allocate(64); }
                
                uint64_t lat = duration_cast<nanoseconds>(t2 - t1).count();
                results[t].push_back(lat);
                asm volatile("" : : "g"(p) : "memory");
            }
        });
    }

    for (auto& t : threads) t.join();

    std::vector<uint64_t> global;
    for (auto& v : results) {
        for (auto lat : v) {
            global.push_back(lat);
            for (auto& [limit, bucket] : histogram) {
                if (lat <= limit) {
                    bucket.count++;
                    break;
                }
            }
        }
    }

    std::sort(global.begin(), global.end());
    
    std::cout << "\nDISTRIBUIÇÃO DE FREQUÊNCIA (HISTOGRAMA):" << std::endl;
    for (auto const& [limit, bucket] : histogram) {
        double pct = (double)bucket.count / global.size() * 100.0;
        std::cout << bucket.label << " | " << std::setw(10) << bucket.count 
                  << " ops | " << std::fixed << std::setprecision(4) << pct << "%" << std::endl;
    }

    std::cout << "\nESTATÍSTICAS DE CAUDA:" << std::endl;
    std::cout << "P99.9   : " << global[global.size() * 0.999] << " ns" << std::endl;
    std::cout << "P99.99  : " << global[global.size() * 0.9999] << " ns" << std::endl;
    std::cout << "Máximo  : " << global.back() / 1000000.0 << " ms" << std::endl;
}

int main() {
    run_audit_session();
    return 0;
}
