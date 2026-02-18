#include <iostream>
#include <fcntl.h>
#include <sys/mman.h>
#include <unistd.h>
#include <iomanip>

int main() {
    const char* filename = "supercore.journal";
    const size_t ARENA_SIZE = 1ULL * 1024 * 1024 * 1024; // 1GB

    int fd = open(filename, O_RDONLY);
    if (fd < 0) { 
        std::cerr << "ERRO: O arquivo supercore.journal nao existe. Rode o ingestor primeiro!" << std::endl;
        return 1; 
    }

    void* data = mmap(nullptr, ARENA_SIZE, PROT_READ, MAP_SHARED, fd, 0);
    if (data == MAP_FAILED) { perror("Erro no mmap"); return 1; }

    unsigned char* ptr = static_cast<unsigned char*>(data);

    std::cout << "\033[1;32m--- INSPECAO DE MEMORIA PERSISTENTE ---\033[0m" << std::endl;
    std::cout << "Primeiros 256 bytes do Journal (Hexdump):" << std::endl;
    
    for (int i = 0; i < 256; ++i) {
        // Destacar o Hex 50 (caractere 'P') em verde
        if (ptr[i] == 0x50) std::cout << "\033[1;32m";
        std::cout << std::hex << std::setw(2) << std::setfill('0') << (int)ptr[i] << " ";
        std::cout << "\033[0m";
        if ((i + 1) % 16 == 0) std::cout << std::endl;
    }

    int p_count = 0;
    // Verifica o primeiro bloco de pacote (1500 bytes)
    for (int i = 0; i < 1500; ++i) {
        if (ptr[i] == 'P') p_count++;
    }

    std::cout << std::dec << "\n--- RESULTADO DA VALIDACAO ---" << std::endl;
    std::cout << "Assinaturas 'P' encontradas no primeiro bloco: " << p_count << " / 1472 esperadas" << std::endl;

    if (p_count > 0) {
        std::cout << "\033[1;32mSTATUS: SUCESSO. O Petronilho Core gravou pacotes integros no NVMe!\033[0m" << std::endl;
    } else {
        std::cout << "\033[1;31mSTATUS: FALHA. Bloco vazio (zeros) ou dados corrompidos.\033[0m" << std::endl;
    }

    munmap(data, ARENA_SIZE);
    close(fd);
    return 0;
}
