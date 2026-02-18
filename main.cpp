#include "super_core.hpp"
#include <iomanip>
#include <iostream>
#include <chrono>

// --- BLOCO DE COMPATIBILIDADE DE REDE ---
#ifdef _WIN32
    #include <winsock2.h>
    #include <ws2tcpip.h>
    #pragma comment(lib, "ws2_32.lib") // Linka a biblioteca de rede do Windows
    typedef int ssize_t;
#else
    #include <netinet/in.h>
    #include <sys/socket.h>
    #include <unistd.h>
    #define INVALID_SOCKET -1
    #define SOCKET_ERROR -1
    typedef int SOCKET;
#endif

int main() {
    // Inicialização necessária apenas para Windows
#ifdef _WIN32
    WSADATA wsaData;
    WSAStartup(MAKEWORD(2, 2), &wsaData);
#endif

    const size_t GIGA = 1024ULL * 1024 * 1024;
    try {
        petronilho::UniversalArena arena("prod_audit.log", 1 * GIGA);
        
        // Uso de SOCKET (funciona em ambos após o ajuste acima)
        SOCKET sockfd = socket(AF_INET, SOCK_DGRAM, 0);
        if (sockfd == INVALID_SOCKET) {
            std::cerr << "Erro ao criar socket" << std::endl;
            return 1;
        }

        sockaddr_in servaddr{};
        servaddr.sin_family = AF_INET;
        servaddr.sin_addr.s_addr = INADDR_ANY;
        servaddr.sin_port = htons(9999);

        if(bind(sockfd, (const struct sockaddr *)&servaddr, sizeof(servaddr)) < 0) {
            std::cerr << "Erro no bind" << std::endl; 
            return 1;
        }

        std::cout << "\033[1;34m[PETRONILHO CORE V5]\033[0m Pronto para Produção." << std::endl;
        petronilho::RecordHeader* header_ptr = nullptr;
        uint64_t count = 0;

        while (true) {
            void* payload_target = arena.allocate(1472, &header_ptr);
            if (!payload_target) break; 

            // recv funciona no Windows e Linux
            ssize_t n = recv(sockfd, (char*)payload_target, 1472, 0); 
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

#ifdef _WIN32
    WSACleanup(); // Finaliza rede no Windows
#endif
    return 0;
}