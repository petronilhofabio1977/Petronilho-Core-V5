#include <iostream>
#include <pthread.h>
#include <liburing.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <fcntl.h>
#include <unistd.h>
#include <cstring>
#include "core/sys/arena_atomic.hpp"
#include "core/sys/network_queue.hpp"
#include "core/sys/telemetry.hpp"

using namespace super_core;

struct NetPacket { 
    uint32_t id; 
    uint64_t timestamp_entry;
    char payload[64]; 
};

typedef net::NetworkQueue<NetPacket, 16384> PacketQueue;
static uint8_t g_arena_mem[64 * 1024 * 1024] __attribute__((aligned(4096)));
static PacketQueue g_queue;
static sys::ScalableArena g_arena(g_arena_mem, sizeof(g_arena_mem));
static bool g_running = true;

void* network_uring_persistence_thread(void* arg) {
    struct io_uring ring;
    io_uring_queue_init(2048, &ring, 0);

    // Socket Setup
    int sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    sockaddr_in servaddr{};
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = INADDR_ANY;
    servaddr.sin_port = htons(9999);
    bind(sockfd, (const struct sockaddr *)&servaddr, sizeof(servaddr));

    // Disk Setup (Log File)
    int log_fd = open("petronilho_network.log", O_WRONLY | O_CREAT | O_APPEND | O_DIRECT, 0644);

    std::cout << "[SYSTEM] Ingest + Persistencia Async Ativa (io_uring)..." << std::endl;

    while (g_running) {
        auto h = g_arena.allocate<NetPacket>();
        if (h.index != 0xFFFFFFFF) {
            NetPacket* p = reinterpret_cast<NetPacket*>(g_arena_mem + h.index);
            
            // 1. Receber da Rede
            struct io_uring_sqe *sqe_net = io_uring_get_sqe(&ring);
            struct iovec iov_net = { .iov_base = p->payload, .iov_len = 64 };
            struct msghdr msg = { .msg_iov = &iov_net, .msg_iovlen = 1 };
            io_uring_prep_recvmsg(sqe_net, sockfd, &msg, 0);
            io_uring_sqe_set_data(sqe_net, (void*)0x1); // Tag para rede

            io_uring_submit(&ring);

            struct io_uring_cqe *cqe;
            if (io_uring_wait_cqe(&ring, &cqe) == 0) {
                if (cqe->res > 0) {
                    p->timestamp_entry = sys::read_tsc();
                    
                    // 2. Agendar Escrita em Disco (Sem travar o CPU)
                    struct io_uring_sqe *sqe_disk = io_uring_get_sqe(&ring);
                    io_uring_prep_write(sqe_disk, log_fd, p->payload, 64, 0);
                    io_uring_sqe_set_data(sqe_disk, (void*)0x2); // Tag para disco
                    io_uring_submit(&ring);

                    // 3. Despachar para Lógica
                    while (!g_queue.enqueue(h.index)) { __builtin_ia32_pause(); }
                }
                io_uring_cqe_seen(&ring, cqe);
            }
        }
    }
    
    close(log_fd);
    io_uring_queue_exit(&ring);
    return nullptr;
}

void* logic_processor_thread(void* arg) {
    uint32_t processed = 0;
    while (g_running) {
        uint32_t idx;
        if (g_queue.dequeue(idx)) {
            NetPacket* p = reinterpret_cast<NetPacket*>(g_arena_mem + idx);
            if (processed % 10000 == 0) {
                std::cout << "[LOGIC] Processado & Persistido batch: " << processed << std::endl;
            }
            processed++;
        }
        if (processed >= 100000) g_running = false; 
    }
    return nullptr;
}

int main() {
    std::memset(g_arena_mem, 0, sizeof(g_arena_mem));
    pthread_t t1, t2;
    pthread_create(&t1, NULL, network_uring_persistence_thread, NULL);
    pthread_create(&t2, NULL, logic_processor_thread, NULL);
    pthread_join(t1, NULL);
    pthread_join(t2, NULL);
    std::cout << "Finalizado com sucesso. Verifique 'petronilho_network.log'." << std::endl;
    return 0;
}
