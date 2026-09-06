#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <unistd.h>
#include <arpa/inet.h>

#include "myUDPConfig.h"

int main()
{
    // Create the UDP socket.
    int sock_fd = socket(myUDPConfig::ADDR_FAMILY, SOCK_DGRAM, IPPROTO_UDP);
    if (sock_fd < 0)
    {
        perror("socket() failed");
        exit(1);
    }

    // Describe the server's address. Notice there is no
    // connect() call before sending in this version - sendto() takes
    // the destination address on every single call. (You can call
    // connect() on a UDP socket to fix the destination and then use
    // plain send()/read(), but doing it explicitly here keeps the
    // "every packet is independent" fundamental visible.)
    sockaddr_in server_addr{};
    server_addr.sin_family = myUDPConfig::ADDR_FAMILY;
    server_addr.sin_port = htons(myUDPConfig::SERVER_PORT);

    // Convert IP string to bytes stored to &server_addr.sin_addr
    if (inet_pton(myUDPConfig::ADDR_FAMILY, myUDPConfig::SERVER_IP, &server_addr.sin_addr) <= 0) {
        perror("inet_pton() failed");
        close(sock_fd);
        exit(1);
    }

    printf("[udp_client] sending to %s:%d\n", myUDPConfig::SERVER_IP, myUDPConfig::SERVER_PORT);
    printf("[udp_client] type a message and press enter. type 'quit' to exit.\n");

    // packet is slightly larger than BUFFER_SIZE to guarantee room for
    // the "<seq>:" prefix on top of a full-length input line, so
    // snprintf below can never be accused of truncating.
    char input[myUDPConfig::BUFFER_SIZE];
    char packet[myUDPConfig::BUFFER_SIZE + sizeof(unsigned int) + 1];
    char buffer[myUDPConfig::BUFFER_SIZE];
    unsigned int seq = 0;

    while (true) {
        printf("> ");
        if (fgets(input, myUDPConfig::BUFFER_SIZE, stdin) == nullptr) {
            break;
        }

        size_t len = strlen(input);
        if (len > 0 && input[len - 1] == '\n') {
            input[len - 1] = '\0';
        }

        if (strcmp(input, "quit") == 0) {
            break;
        }

        // Tag the message with a sequence number before sending.
        snprintf(packet, sizeof(packet), "%u:%s", seq, input);
        seq++;

        socklen_t server_len = sizeof(server_addr);
        ssize_t bytes_sent = sendto(
            sock_fd, packet, strlen(packet), 0,
            (sockaddr*)&server_addr, server_len
        );
        if (bytes_sent < 0) {
            perror("sendto() failed");
            continue;
        }

        // Wait for the server's reply. Unlike TCP, there is no
        // guarantee this will ever arrive - if it's lost, recvfrom()
        // just blocks forever (a real system would add a timeout here;
        // deliberately left out so the "no guarantee" fundamental is
        // visible rather than papered over).
        memset(buffer, 0, myUDPConfig::BUFFER_SIZE);
        sockaddr_in from_addr;
        socklen_t from_len = sizeof(from_addr);
        ssize_t bytes_received = recvfrom(
            sock_fd, buffer, myUDPConfig::BUFFER_SIZE - 1, 0,
            (sockaddr*)&from_addr, &from_len
        );

        if (bytes_received < 0) {
            perror("recvfrom() failed");
            continue;
        }

        printf("[udp_client] echo: %s\n", buffer);
    }

    close(sock_fd);
    printf("[udp_client] done\n");
    return 0;
}