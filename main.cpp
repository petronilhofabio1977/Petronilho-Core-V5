#include "super_core.hpp"
#include <netinet/in.h>
#include <sys/socket.h>
#include <iomanip>

int main() {
    const size_t GIGA = 1024ULL * 1024 * 1024;
    try {
        petronilho::UniversalArena arena("prod_audit.log", 1 * GIGA);
        int sockfd = socket(AF_INET, SOCK_DGRAM, 0);
        sockaddr_in servaddr{AF_INET, htons(9999), {INADDR_ANY}};
        if(bind(sockfd, (const struct sockaddr *)&servaddr, sizeof(servaddr)) < 0) {
            perror("Erro no bind"); return 1;
        }

        std::cout << "\033[1;34m[PETRONILHO CORE V5]\033[0m Pronto para Produção." << std::endl;
        RecordHeader* header_ptr = nullptr;
        uint64_t count = 0;

        while (true) {
            void* payload_target = arena.allocate(1472, &header_ptr);
            if (!payload_target) break; 

            ssize_t n = recv(sockfd, payload_target, 1472, 0);
            if (n > 0) {
                header_ptr->timestamp_ns = std::chrono::high_resolution_clock::now().time_since_epoch().count();
                header_ptr->payload_len = static_cast<uint32_t>(n);
                header_ptr->magic_id = 0xDEADBEEF;
                if (++count % 50000 == 0) {
                    std::cout << "\r[MONITOR] Pacotes: " << count 
                              << " | Offset: " << (arena.get_offset() / (1024 * 1024)) << " MB" << std::flush;
                }
            }
        }
    } catch (const std::exception& e) {
        std::cerr << "Falha: " << e.what() << std::endl;
    }
    return 0;
}
