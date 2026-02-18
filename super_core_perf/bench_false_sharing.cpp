#include <iostream>
#include <atomic>
#include <thread>
#include <vector>
#include <chrono>
struct CounterSimple { std::atomic<uint64_t> value{0}; };
struct alignas(64) CounterPadded { std::atomic<uint64_t> value{0}; };
template<typename T>
void bench(const std::string& label) {
    const int threads_n = 4; const uint64_t iters = 100000000;
    std::vector<T> counters(threads_n); std::vector<std::thread> threads;
    auto start = std::chrono::steady_clock::now();
    for (int i = 0; i < threads_n; ++i) {
        threads.emplace_back([&counters, i, iters]() {
            for (uint64_t j = 0; j < iters; ++j) counters[i].value.fetch_add(1, std::memory_order_relaxed);
        });
    }
    for (auto& t : threads) t.join();
    auto end = std::chrono::steady_clock::now();
    std::cout << label << ": " << std::chrono::duration_cast<std::chrono::milliseconds>(end-start).count() << " ms" << std::endl;
}
int main() { bench<CounterSimple>("Unpadded"); bench<CounterPadded>("Padded (64B)"); return 0; }
