#include "super_core.hpp"
#include <iomanip>
#include <iostream>
#include <chrono>
#include <vector>
#include <fstream>
#include <thread>
#include <atomic>
#include <fcntl.h>

// --- BLOCO DE COMPATIBILIDADE DE REDE E SISTEMA ---
#ifdef _WIN32
    #include <winsock2.h>
    #include <ws2tcpip.h>
    #include <windows.h> // <--- ESSENCIAL PARA TRAVAR O CORE NO WINDOWS
    #pragma comment(lib, "ws2_32.lib")
    typedef int ssize_t;
#else
    #include <netinet/in.h>
    #include <sys/socket.h>
    #include <unistd.h>
    #include <sched.h> 
    #define INVALID_SOCKET -1
    typedef int SOCKET;
#endif

struct PerfEntry {
    uint32_t packet_id;
    uint64_t ts_ns;
    uint32_t latency_ns;
};

int main() {
#ifdef _WIN32
    // 1. Inicializa Rede no Windows
    WSADATA wsaData; WSAStartup(MAKEWORD(2, 2), &wsaData);
    
    // 2. Trava no Core 2 no Windows (Afinidade)
    HANDLE thread = GetCurrentThread();
    DWORD_PTR mask = (static_cast<DWORD_PTR>(1) << 2); // Bit 2 = Core 2
    SetThreadAffinityMask(thread, mask);
    std::cout << "[WIN32] Afinidade definida para Core 2." << std::endl;
#else
    // Trava no Core 2 no Linux
    cpu_set_t cpuset; CPU_ZERO(&cpuset); CPU_SET(2, &cpuset);
    sched_setaffinity(0, sizeof(cpu_set_t), &cpuset);
    std::cout << "[LINUX] Afinidade definida para Core 2." << std::endl;
#endif

    const size_t GIGA = 1024ULL * 1024 * 1024;
    const uint64_t MAX_LOG = 5000000;
    std::vector<PerfEntry> log_buffer;
    log_buffer.reserve(MAX_LOG);

    try {
        // Nome do log ajustado para Windows/Linux
        petronilho::UniversalArena arena("prod_audit.log", 2 * GIGA); 
        SOCKET sockfd = socket(AF_INET, SOCK_DGRAM, 0);
        
#ifdef _WIN32
        u_long mode = 1; ioctlsocket(sockfd, FIONBIO, &mode);
#else
        fcntl(sockfd, F_SETFL, fcntl(sockfd, F_GETFL, 0) | O_NONBLOCK);
#endif

        sockaddr_in servaddr{};
        servaddr.sin_family = AF_INET;
        servaddr.sin_addr.s_addr = INADDR_ANY;
        servaddr.sin_port = htons(9999);
        bind(sockfd, (const struct sockaddr *)&servaddr, sizeof(servaddr));

        std::cout << "\033[1;32m[PETRONILHO CORE V5]\033[0m Benchmark de 5 min ativo." << std::endl;

        uint64_t count = 0;
        auto start_bench = std::chrono::steady_clock::now();

        while (true) {
            auto now = std::chrono::steady_clock::now();
            auto elapsed = std::chrono::duration_cast<std::chrono::seconds>(now - start_bench).count();
            
            if (elapsed >= 300) break; 

            RecordHeader* header_ptr = nullptr;
            void* payload_target = arena.allocate(1472, &header_ptr);
            if (!payload_target) break;

            auto t1 = std::chrono::high_resolution_clock::now();
            ssize_t n = recv(sockfd, (char*)payload_target, 1472, 0);
            auto t2 = std::chrono::high_resolution_clock::now();

            if (n > 0) {
                uint32_t lat = (uint32_t)std::chrono::duration_cast<std::chrono::nanoseconds>(t2 - t1).count();
                if (count < MAX_LOG) {
                    log_buffer.push_back({(uint32_t)count, (uint64_t)t2.time_since_epoch().count(), lat});
                }
                if (++count % 100000 == 0) {
                    std::cout << "\r[MONITOR] Tempo: " << elapsed << "s | Pacotes: " << count << std::flush;
                }
            }
        }

        std::cout << "\n\n[FINALIZANDO] Gravando registros no CSV..." << std::endl;
        // Nome do arquivo diferente para não sobrescrever o do Linux se você usar a mesma pasta
        std::ofstream csv("petronilho_audit_win_5min.csv");
        csv << "ID,TS_NS,LAT_NS\n";
        for (const auto& entry : log_buffer) {
            csv << entry.packet_id << "," << entry.ts_ns << "," << entry.latency_ns << "\n";
        }
        std::cout << "[SUCESSO] Relatório Windows gerado!" << std::endl;

    } catch (const std::exception& e) { std::cerr << "Falha: " << e.what() << std::endl; }

#ifdef _WIN32
    WSACleanup();
#endif
    return 0;
}