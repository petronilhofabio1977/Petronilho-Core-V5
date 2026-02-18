#include <iostream>
#include <fcntl.h>
#include <sys/mman.h>
#include <unistd.h>
#include <cstdint>
#include <cstring>

struct RecordHeader {
    uint64_t timestamp_ns;
    uint32_t payload_len;
    uint32_t magic;
};

int main() {
    int fd = open("universal_audit.log", O_RDONLY);
    if (fd < 0) return 1;
    size_t size = lseek(fd, 0, SEEK_END);
    uint8_t* ptr = (uint8_t*)mmap(nullptr, size, PROT_READ, MAP_SHARED, fd, 0);
    
    // Varre a arena em busca do primeiro registro com o nosso Magic Number
    RecordHeader* h = (RecordHeader*)ptr;
    
    if (h->magic == 0xDEADBEEF) {
        char content[1024] = {0};
        memcpy(content, ptr + sizeof(RecordHeader), h->payload_len);
        
        std::cout << "\n\033[1;36m>> DATA DASHBOARD <<\033[0m" << std::endl;
        std::cout << "------------------------------------" << std::endl;
        std::cout << "STATUS:   [\033[1;32mGRAVADO COM SUCESSO\033[0m]" << std::endl;
        std::cout << "TIME:     [" << h->timestamp_ns << " ns]" << std::endl;
        std::cout << "MAGIC:    [0x" << std::hex << h->magic << std::dec << "]" << std::endl;
        std::cout << "PAYLOAD:  [\033[1;33m" << content << "\033[0m]" << std::endl;
        std::cout << "------------------------------------" << std::endl;
    } else {
        std::cout << "\033[1;31m[ERRO] Nenhum dado integro encontrado no log.\033[0m" << std::endl;
    }
    munmap(ptr, size);
    close(fd);
    return 0;
}
