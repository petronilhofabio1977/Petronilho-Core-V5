#include "super_core_universal.hpp"
#include <iostream>
#include <cstring>

int main() {
    try {
        const size_t ARENA_SIZE = 1024 * 1024 * 100; // 100MB
        petronilho::UniversalArena arena("universal_audit.log", ARENA_SIZE);
        
        std::cout << "[SUCCESS] Arena Universal alocada com sucesso!" << std::endl;
        
        RecordHeader* h = nullptr;
        const char* mensagem = "PETRONILHO_CORE_V5_ATIVADO";
        uint32_t msg_len = strlen(mensagem);

        // Aloca espaço na Arena
        void* data = arena.allocate(msg_len, &h);
        
        if (data) {
            // ESCREVE O DADO REAL NA MEMÓRIA MAPEADA (Zero-Copy)
            memcpy(data, mensagem, msg_len);
            
            h->timestamp_ns = std::chrono::high_resolution_clock::now().time_since_epoch().count();
            h->payload_len = msg_len;
            h->magic = 0xDEADBEEF;
            
            std::cout << "[LOG] Registro de teste criado com sucesso!" << std::endl;
        }

    } catch (const std::exception& e) {
        std::cerr << "ERRO: " << e.what() << std::endl;
        return 1;
    }
    return 0;
}
