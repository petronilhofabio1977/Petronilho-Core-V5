#include "arena.hpp"
#include <iostream>
#include <vector>
#include <thread>
#include <atomic>
#include <chrono>
#include <cstring>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>
#include <fcntl.h>

using namespace std::chrono;

const int PORT = 9999;
const size_t PACKET_SIZE = 1500;
const size_t ARENA_SIZE = 512ULL * 1024 * 1024;

std::atomic<bool> running{true};
std::atomic<uint64_t> packets_received{0};
std::atomic<uint64_t> bytes_received{0};

void network_ingest_worker(petronilho::ScalableArena* arena) {
    int sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd < 0) return;

    int flags = fcntl(sockfd, fcntl(sockfd, F_GETFL, 0) | O_NONBLOCK);
    
    sockaddr_in servaddr{};
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = INADDR_ANY;
    servaddr.sin_port = htons(PORT);

    bind(sockfd, (const struct sockaddr *)&servaddr, sizeof(servaddr));

    while (running.load(std::memory_order_relaxed)) {
        void* buffer = arena->allocate(PACKET_SIZE);
        if (!buffer) { arena->reset(); buffer = arena->allocate(PACKET_SIZE); }

        ssize_t n = recv(sockfd, buffer, PACKET_SIZE, 0);
        if (n > 0) {
            packets_received.fetch_add(1, std::memory_order_relaxed);
            bytes_received.fetch_add(n, std::memory_order_relaxed);
        }
    }
    close(sockfd);
}

int main() {
    petronilho::ScalableArena arena(ARENA_SIZE);
    std::cout << "[INGESTOR] Petronilho Core Ativo na porta " << PORT << std::endl;
    
    std::thread ingest_thread(network_ingest_worker, &arena);

    auto start = steady_clock::now();
    for(int i=0; i<10; ++i) {
        std::this_thread::sleep_for(seconds(1));
        double gbps = (bytes_received.load() * 8.0) / (1e9);
        std::cout << "[MONITOR] Pacotes: " << packets_received.load() 
                  << " | Dados: " << std::fixed << std::setprecision(2) << gbps << " Gb acumulados" << std::endl;
    }

    running = false;
    ingest_thread.join();
    return 0;
}
