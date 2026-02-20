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
#include <fstream>   // Necessário para o LOG
#include <iomanip>

using namespace std::chrono;

// --- ESTRUTURA DE AUDITORIA ---
struct PerfMetrics {
    uint64_t seq;
    uint64_t ts_ns;
    uint64_t latency_ns;
};

const int PORT = 9999;
const size_t PACKET_SIZE = 1500;
const size_t ARENA_SIZE = 512ULL * 1024 * 1024;
const size_t MAX_LOG_ENTRIES = 1000000; // Log de 1 milhão de pacotes

std::atomic<bool> running{true};
std::atomic<uint64_t> packets_received{0};
std::atomic<uint64_t> bytes_received{0};

// Buffer global para não interferir na Arena de dados
PerfMetrics* telemetry_log = new PerfMetrics[MAX_LOG_ENTRIES];

void network_ingest_worker(petronilho::ScalableArena* arena) {
    int sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd < 0) return;

    // Otimização de Socket
    int flags = fcntl(sockfd, F_GETFL, 0);
    fcntl(sockfd, F_SETFL, flags | O_NONBLOCK);
    
    sockaddr_in servaddr{};
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = INADDR_ANY;
    servaddr.sin_port = htons(PORT);

    if (bind(sockfd, (const struct sockaddr *)&servaddr, sizeof(servaddr)) < 0) return;

    while (running.load(std::memory_order_relaxed)) {
        void* buffer = arena->allocate(PACKET_SIZE);
        if (!buffer) { arena->reset(); buffer = arena->allocate(PACKET_SIZE); }

        // --- INÍCIO DA AUDITORIA ---
        auto t1 = high_resolution_clock::now();

        ssize_t n = recv(sockfd, buffer, PACKET_SIZE, 0);
        
        if (n > 0) {
            auto t2 = high_resolution_clock::now();
            uint64_t lat = duration_cast<nanoseconds>(t2 - t1).count();
            uint64_t current_idx = packets_received.load(std::memory_order_relaxed);

            // Grava no log de memória (O(1) complexity)
            if (current_idx < MAX_LOG_ENTRIES) {
                telemetry_log[current_idx] = { current_idx, (uint64_t)t2.time_since_epoch().count(), lat };
            }

            packets_received.fetch_add(1, std::memory_order_relaxed);
            bytes_received.fetch_add(n, std::memory_order_relaxed);
        }
    }
    close(sockfd);
}

int main() {
    petronilho::ScalableArena arena(ARENA_SIZE);
    std::cout << "[INGESTOR] Petronilho Core Ativo | Auditoria: Ativa (1M registros)" << std::endl;
    
    std::thread ingest_thread(network_ingest_worker, &arena);

    // Monitor de console (Dono do dinheiro gosta de ver movimento)
    for(int i=0; i<10; ++i) {
        std::this_thread::sleep_for(seconds(1));
        double gbps = (bytes_received.load() * 8.0) / (1e9);
        std::cout << "[MONITOR] Pacotes: " << packets_received.load() 
                  << " | Taxa: " << std::fixed << std::setprecision(2) << gbps << " Gb acumulados" << std::endl;
    }

    std::cout << "\n[SISTEMA] Finalizando e gerando relatório CSV..." << std::endl;
    running = false;
    ingest_thread.join();

    // --- GERAÇÃO DO CSV DE AUDITORIA ---
    std::ofstream csv("petronilho_audit.csv");
    csv << "Sequencia,Timestamp_NS,Latencia_NS\n";
    uint64_t total = std::min(packets_received.load(), MAX_LOG_ENTRIES);
    
    for(uint64_t i = 0; i < total; ++i) {
        csv << telemetry_log[i].seq << "," 
            << telemetry_log[i].ts_ns << "," 
            << telemetry_log[i].latency_ns << "\n";
    }
    csv.close();

    std::cout << "[SUCESSO] Arquivo 'petronilho_audit.csv' gerado com " << total << " registros." << std::endl;

    delete[] telemetry_log;
    return 0;
}