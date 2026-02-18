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
#include <random>
#include <thread>
#include <cstdlib>

namespace petronilho::hpc {

    // ============================================================
    //  Compiler Barrier (anti DCE)
    // ============================================================

    template<typename T>
    inline void do_not_optimize(const T& value) {
        asm volatile("" : : "g"(value) : "memory");
    }

    // ============================================================
    //  CPUID Full Barrier
    // ============================================================

    inline void full_barrier() {
        asm volatile(
            "cpuid"
            :
            :
            : "rax", "rbx", "rcx", "rdx", "memory"
        );
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

    // ============================================================
    //  CPU Affinity
    // ============================================================

    void set_cpu_affinity(int cpu_id = 0) {
        cpu_set_t cpuset;
        CPU_ZERO(&cpuset);
        CPU_SET(cpu_id, &cpuset);

        if (sched_setaffinity(0, sizeof(cpu_set_t), &cpuset) != 0) {
            std::cerr << "Warning: CPU affinity failed.\n";
        }
    }

    // ============================================================
    //  Invariant TSC Check (GCC-safe)
    // ============================================================

    bool has_invariant_tsc() {
        uint32_t a, b, c, d;

        unsigned int max_leaf = __get_cpuid_max(0x80000000, nullptr);
        if (max_leaf < 0x80000007)
            return false;

        if (!__get_cpuid(0x80000007, &a, &b, &c, &d))
            return false;

        return (d & (1u << 8)) != 0;
    }

    // ============================================================
    //  Frequency Estimation
    // ============================================================

    double estimate_ghz() {
        full_barrier();
        uint64_t t1 = __rdtsc();
        auto c1 = std::chrono::steady_clock::now();

        std::this_thread::sleep_for(std::chrono::milliseconds(200));

        full_barrier();
        uint64_t t2 = __rdtsc();
        auto c2 = std::chrono::steady_clock::now();

        double seconds =
            std::chrono::duration<double>(c2 - c1).count();

        return (t2 - t1) / seconds / 1e9;
    }

    // ============================================================
    //  Arena Allocator
    // ============================================================

    class ScalableArena {
        uint8_t* memory;
        size_t capacity;
        std::atomic<size_t> offset;

    public:
        explicit ScalableArena(size_t size, bool touch_pages = true)
            : offset(0)
        {
            const size_t alignment = 4096;
            capacity = (size + alignment - 1) & ~(alignment - 1);

            if (posix_memalign(
                    reinterpret_cast<void**>(&memory),
                    alignment,
                    capacity) != 0)
            {
                std::abort();
            }

            if (touch_pages)
                std::memset(memory, 0, capacity);
        }

        ~ScalableArena() {
            std::free(memory);
        }

        inline void* allocate(size_t size) {
            size_t aligned =
                (size + 15) & ~static_cast<size_t>(15);

            size_t current =
                offset.fetch_add(aligned,
                                 std::memory_order_relaxed);

            if (current + aligned > capacity)
                return nullptr;

            return memory + current;
        }

        void reset() {
            offset.store(0, std::memory_order_relaxed);
        }
    };

    // ============================================================
    //  Statistics
    // ============================================================

    struct Stats {
        std::string name;
        std::vector<uint64_t> samples;

        void report(double ghz) {
            if (samples.empty()) return;

            std::sort(samples.begin(), samples.end());

            double mean =
                std::accumulate(samples.begin(),
                                samples.end(),
                                0.0)
                / samples.size();

            size_t idx =
                static_cast<size_t>(
                    0.99 * (samples.size() - 1));

            uint64_t p99 = samples[idx];

            std::cout << std::left
                      << std::setw(32) << name
                      << " | Mean: "
                      << std::setw(10)
                      << mean << " cyc ("
                      << mean / ghz << " ns)"
                      << " | P99: "
                      << p99
                      << " cyc\n";
        }
    };

} // namespace petronilho::hpc

// ============================================================
//  MAIN
// ============================================================

int main() {

    using namespace petronilho::hpc;

    set_cpu_affinity(0);

    std::cout << "--- PETRONILHO Academic Suite v5 ---\n";
    std::cout << "Invariant TSC: "
              << (has_invariant_tsc() ? "Yes" : "No")
              << "\n";

    double ghz = estimate_ghz();
    std::cout << "Estimated GHz: "
              << ghz
              << "\n";
    std::cout << "------------------------------------\n";

    // ========================================================
    //  1) Latency Benchmark
    // ========================================================

    {
        const int batches = 10;
        const int iters = 100000;
        const int warmup = 50000;

        ScalableArena arena(256ull * 1024 * 1024);

        for (int i = 0; i < warmup; ++i) {
            void* p = arena.allocate(64);
            do_not_optimize(p);
        }
        arena.reset();

        Stats stats{ "Arena Allocate 64B" };
        stats.samples.reserve(batches * iters);

        for (int b = 0; b < batches; ++b) {
            arena.reset();

            for (int i = 0; i < iters; ++i) {
                uint64_t t = rdtsc_start();
                void* p = arena.allocate(64);
                uint64_t e = rdtsc_end();

                if (!p) std::abort();

                do_not_optimize(p);
                stats.samples.push_back(e - t);
            }
        }

        stats.report(ghz);
    }

    return 0;
}