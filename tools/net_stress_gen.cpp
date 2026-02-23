#include <iostream>
#include <cstring>
#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>

int main() {
    int sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    sockaddr_in servaddr{};
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(9999);
    servaddr.sin_addr.s_addr = inet_addr("127.0.0.1");
    char payload[1472];
    std::memset(payload, 'P', sizeof(payload));
    
    std::cout << "[STRESSOR] Iniciando flood UDP..." << std::endl;
    while (true) {
        sendto(sockfd, payload, sizeof(payload), 0, (const struct sockaddr *)&servaddr, sizeof(servaddr));
    }
    return 0;
}
