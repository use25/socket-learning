#include <cstdio>      // printf, perror
#include <cstdlib>     // exit
#include <cstring>     // memset
#include <unistd.h>    // close, read, write
#include <arpa/inet.h> // sockaddr_in, htons, INADDR_ANY

#include "myTCPConfig.h"
#include "TCPHelper.h"

int main() {
    // Forces log output to appear immediately instead of being buffered
    setvbuf(stdout, nullptr, _IOLBF, 0);

    // Create socket IPv4 TCP
    int server_fd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (server_fd < 0) {
        perror("socket() failed");
        exit(1);
    }
    printf("[server] socket created, fd = %d\n", server_fd);

    // Allow reusing the port immediately after the program exits.
    // Without this, restarting the server quickly often fails with
    // "Address already in use" because the OS keeps the port in a
    // TIME_WAIT state for a while after close().
    int opt = 1;
    setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    // Build the address
    sockaddr_in server_addr;
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;     // listen on all local interfaces
    server_addr.sin_port = htons(myTCPConfig::SERVER_PORT);    // htons = host-to-network byte order
    printf("[server] port %d htons: %d\n", myTCPConfig::SERVER_PORT, server_addr.sin_port);

    // bind() tells the OS "any traffic to this IP:port should come to me."
    if (bind(server_fd, (sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        perror("bind() failed");
        close(server_fd);
        exit(1);
    }
    printf("[server] bound to port %d\n", myTCPConfig::SERVER_PORT);

    if (listen(server_fd, myTCPConfig::BACKLOG) < 0) {
        perror("listen() failed");
        close(server_fd);
        exit(1);
    }
    printf("[server] listening... waiting for a client\n");

    // accept() blocks until a client connects. It returns a
    // new socket file descriptor dedicated to that one client.
    // The original server_fd keeps listening for other clients
    sockaddr_in client_addr;
    socklen_t client_len = sizeof(client_addr);
    int client_fd = accept(server_fd, (sockaddr*)&client_addr, &client_len);
    if (client_fd < 0) {
        perror("accept() failed");
        close(server_fd);
        exit(1);
    }

    // Convert the client's binary IP address into a human-readable string.
    char client_ip[INET_ADDRSTRLEN];
    inet_ntop(AF_INET, &client_addr.sin_addr, client_ip, sizeof(client_ip));
    printf("[server] client connected: %s:%d\n", client_ip, ntohs(client_addr.sin_port));

    char buffer[myTCPConfig::BUFFER_SIZE];
    memset(buffer, 0, myTCPConfig::BUFFER_SIZE);
    while (true) {
        size_t len_from_client;
        TCPHelper::ReturnCode result;

        result = TCPHelper::ReadN(client_fd, myTCPConfig::BUFFER_SIZE, (void*)buffer, len_from_client);
        if (result < TCPHelper::ReturnCode::SUCCESS)
        {
            perror("[server] read from client failed");
            break;
        }

        printf("[server] Received message from client: %s\n", buffer);

        // Sending back exactly the same buffer/message to client for now
        result = TCPHelper::WriteN(client_fd, (const void*)buffer, len_from_client);
        if (result < TCPHelper::ReturnCode::SUCCESS)
        {
            perror("[server] write to client failed");
            break;
        }

        // Clean up before next read session
        memset(buffer, 0, len_from_client);
    }

    close(client_fd);
    close(server_fd);
    printf("[server] shut down cleanly\n");
    return 0;
}