#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>
#include <sched.h>
#include <fcntl.h>
#include "ring_buffer.hpp"
#include <iostream>
#include <chrono>
#include <fstream>

int main() {
    cpu_set_t cpuset;
    CPU_ZERO(&cpuset);
    CPU_SET(2, &cpuset);
    sched_setaffinity(0, sizeof(cpu_set_t), &cpuset);
    std::cout << "[LINUX] Afinidade: Core 2." << std::endl;

    try {
        petronilho::RingBuffer ring("ring_audit.bin", 65536);

        int sockfd = socket(AF_INET, SOCK_DGRAM, 0);
        fcntl(sockfd, F_SETFL, fcntl(sockfd, F_GETFL, 0) | O_NONBLOCK);

        sockaddr_in servaddr{};
        servaddr.sin_family      = AF_INET;
        servaddr.sin_addr.s_addr = INADDR_ANY;
        servaddr.sin_port        = htons(9999);
        bind(sockfd, (const struct sockaddr*)&servaddr, sizeof(servaddr));

        std::cout << "[PETRONILHO CORE V5] Ring Buffer ativo. Capacidade: "
                  << ring.capacity() << " slots." << std::endl;

        char recv_buf[1472];
        uint64_t count   = 0;
        uint64_t dropped = 0;

        auto start = std::chrono::steady_clock::now();

        while (true) {
            auto now     = std::chrono::steady_clock::now();
            auto elapsed = std::chrono::duration_cast<std::chrono::seconds>(now - start).count();
            if (elapsed >= 300) break;

            auto t1  = std::chrono::high_resolution_clock::now();
            ssize_t n = recv(sockfd, recv_buf, sizeof(recv_buf), 0);
            auto t2  = std::chrono::high_resolution_clock::now();

            if (n > 0) {
                uint32_t lat = (uint32_t)std::chrono::duration_cast<std::chrono::nanoseconds>(t2 - t1).count();
                uint64_t ts  = (uint64_t)t2.time_since_epoch().count();

                if (!ring.write(ts, lat, (uint32_t)count, recv_buf, (uint32_t)n))
                    dropped++;

                if (++count % 100000 == 0) {
                    std::cout << "\r[MONITOR] " << elapsed << "s | Pacotes: " << count
                              << " | Buffer: " << ring.size() << "/" << ring.capacity()
                              << " | Descartados: " << dropped << std::flush;
                }
            }
        }

        std::cout << "\n[FINALIZANDO] Exportando CSV..." << std::endl;
        std::ofstream csv("petronilho_ring_5min.csv");
        csv << "ID,TS_NS,LAT_NS\n";

        petronilho::RingSlot slot;
        uint64_t exported = 0;
        while (ring.read(slot)) {
            csv << slot.packet_id << "," << slot.timestamp_ns << "," << slot.latency_ns << "\n";
            exported++;
        }

        std::cout << "[SUCESSO] Processado: " << count   << std::endl;
        std::cout << "[SUCESSO] Exportado : " << exported << std::endl;
        std::cout << "[SUCESSO] Descartados: " << dropped  << std::endl;

        close(sockfd);

    } catch (const std::exception& e) {
        std::cerr << "Falha: " << e.what() << std::endl;
    }
    return 0;
}
