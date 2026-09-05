#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <unistd.h>
#include <arpa/inet.h>

#include "myTCPConfig.h"
#include "TCPHelper.h"

int main() {
    // Create socket IPv4, TCP.
    int sock_fd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (sock_fd < 0) {
        perror("socket() failed");
        exit(1);
    }

    // Build address of the server where client wants to go
    sockaddr_in server_addr;
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(myTCPConfig::SERVER_PORT);

    // inet_pton converts string IP into bytes for kernel
    if (inet_pton(AF_INET, myTCPConfig::SERVER_IP, &server_addr.sin_addr) <= 0) {
        perror("inet_pton() failed - invalid address");
        close(sock_fd);
        exit(1);
    }

    // TCP three-way handshake (SYN, SYN-ACK, ACK). It blocks until the connection is established or fails.
    if (connect(sock_fd, (sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        perror("connect() failed - is the server running?");
        close(sock_fd);
        exit(1);
    }
    printf("[client] connected to %s:%d\n", myTCPConfig::SERVER_IP, myTCPConfig::SERVER_PORT);
    printf("[client] type a message and press enter. type 'quit' to exit.\n");

    // Send/receive loop.
    char input[myTCPConfig::BUFFER_SIZE];
    char buffer_server[myTCPConfig::BUFFER_SIZE];
    memset(input, 0, myTCPConfig::BUFFER_SIZE);
    memset(buffer_server, 0, myTCPConfig::BUFFER_SIZE);

    while (true) {
        printf("> ");
        if (fgets(input, myTCPConfig::BUFFER_SIZE, stdin) == nullptr) {
            break; // e.g. Ctrl+D
        }

        size_t len = strlen(input);
        if (len > 0 && input[len - 1] == '\n') {
            input[len - 1] = '\0';
            len--;
        }

        if (strcmp(input, "quit") == 0) {
            break;
        }

        TCPHelper::ReturnCode result;

        result = TCPHelper::WriteN(sock_fd, (const void*)input, len);
        if (result < TCPHelper::ReturnCode::SUCCESS)
        {
            perror("[client] write to server failed");
            break;
        }

        size_t len_from_server;
        result = TCPHelper::ReadN(sock_fd, myTCPConfig::BUFFER_SIZE, (void*)buffer_server, len_from_server);
        if (result < TCPHelper::ReturnCode::SUCCESS)
        {
            perror("[client] read from server failed");
            break;
        }

        printf("[client] Received from server with size: %zd\n", len_from_server);
        printf("[client] Received from server: %s\n", buffer_server);
        // Clean up before next read session
        memset(buffer_server, 0, len_from_server);
    }

    close(sock_fd);
    printf("[client] disconnected\n");
    return 0;
}