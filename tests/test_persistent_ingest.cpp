#include "persistent_arena.hpp"
#include <iostream>
#include <thread>
#include <atomic>
#include <chrono>
#include <netinet/in.h>
#include <sys/socket.h>
#include <iomanip>

int main() {
    // 1GB de Journal Persistente mapeado em disco
    const size_t ARENA_SIZE = 1ULL * 1024 * 1024 * 1024; 
    petronilho::PersistentArena arena("supercore.journal", ARENA_SIZE);

    int sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd < 0) { perror("Socket erro"); return 1; }

    sockaddr_in servaddr{};
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = INADDR_ANY;
    servaddr.sin_port = htons(9999);

    if (bind(sockfd, (const struct sockaddr *)&servaddr, sizeof(servaddr)) < 0) {
        perror("Bind erro"); return 1;
    }

    std::cout << "[PERSISTENCE] Petronilho Core gravando em supercore.journal..." << std::endl;

    std::atomic<uint64_t> packets{0};
    std::atomic<uint64_t> bytes{0};
    
    std::thread monitor([&]() {
        auto start = std::chrono::steady_clock::now();
        while(true) {
            std::this_thread::sleep_for(std::chrono::seconds(1));
            double gb_total = (bytes.load() * 8.0) / 1e9;
            std::cout << "[MONITOR] Pacotes no NVMe: " << packets.load() 
                      << " | Escrito: " << std::fixed << std::setprecision(2) << gb_total << " Gb" << std::endl;
        }
    });
    monitor.detach();

    while (true) {
        void* buffer = arena.allocate(1500);
        if (!buffer) {
            std::cout << "[AVISO] Arena Persistente Cheia. Reiniciando..." << std::endl;
            arena.reset();
            buffer = arena.allocate(1500);
        }

        ssize_t n = recv(sockfd, buffer, 1500, 0);
        if (n > 0) {
            packets.fetch_add(1, std::memory_order_relaxed);
            bytes.fetch_add(n, std::memory_order_relaxed);
        }
    }

    return 0;
}
