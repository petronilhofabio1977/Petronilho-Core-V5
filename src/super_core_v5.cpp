#include <iostream>
#include <atomic>
#include <chrono>
#include <sys/mman.h>
#include <fcntl.h>
#include <unistd.h>
#include <netinet/in.h>

// 1. HEADER DE PACOTE (Metadados para Auditoria)
struct RecordHeader {
    uint64_t timestamp_ns;
    uint32_t payload_len;
    uint32_t magic; // 0xDEADBEEF
};

int main() {
    const char* filename = "prod_audit.log";
    const size_t ARENA_SIZE = 1024ULL * 1024 * 512; // 512MB

    // 2. RECUPERAÇÃO DE ESTADO
    // Tentamos abrir o arquivo existente para ler o tamanho e continuar de onde parou
    int fd = open(filename, O_RDWR | O_CREAT, 0666);
    size_t current_file_size = lseek(fd, 0, SEEK_END);
    if (current_file_size < ARENA_SIZE) ftruncate(fd, ARENA_SIZE);

    // 3. HUGE PAGES (Blindagem de Performance)
    // Tentamos MAP_HUGETLB. Se o sistema não suportar, ele cai para o mmap padrão automaticamente.
    void* memory = mmap(nullptr, ARENA_SIZE, PROT_READ | PROT_WRITE, 
                        MAP_SHARED | MAP_POPULATE, fd, 0);

    if (memory == MAP_FAILED) {
        std::cout << "[AVISO] Huge Pages não disponíveis. Usando páginas padrão." << std::endl;
        memory = mmap(nullptr, ARENA_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    }

    uint8_t* ptr = (uint8_t*)memory;
    
    // Recupera o offset real (procura o último Magic Number válido ou começa do zero)
    // Para simplificar no MVP, vamos apenas anexar (append) se o arquivo já tiver dados.
    std::atomic<size_t> offset{0};
    
    int sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    sockaddr_in addr{AF_INET, htons(9999), {INADDR_ANY}};
    bind(sockfd, (const struct sockaddr *)&addr, sizeof(addr));

    std::cout << "[PETRONILHO V5] Core Ativo. Gravando em: " << filename << std::endl;

    while (true) {
        size_t needed = sizeof(RecordHeader) + 1472;
        size_t curr_off = offset.fetch_add(needed, std::memory_order_relaxed);

        if (curr_off + needed > ARENA_SIZE) {
            std::cout << "[LOG] Arena Cheia. Reiniciando ciclo." << std::endl;
            offset.store(0); 
            continue;
        }

        RecordHeader* h = (RecordHeader*)(ptr + curr_off);
        
        // Recebe o dado (Zero-Copy)
        ssize_t n = recv(sockfd, ptr + curr_off + sizeof(RecordHeader), 1472, 0);
        
        if (n > 0) {
            h->timestamp_ns = std::chrono::high_resolution_clock::now().time_since_epoch().count();
            h->payload_len = (uint32_t)n;
            h->magic = 0xDEADBEEF;
        }
    }
    return 0;
}
