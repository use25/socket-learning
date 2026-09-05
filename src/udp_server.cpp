#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <unistd.h>
#include <arpa/inet.h>

#include "myUDPConfig.h"

int main() {
    //setvbuf(stdout, nullptr, _IOLBF, 0);

    int sock_fd = socket(myUDPConfig::ADDR_FAMILY, SOCK_DGRAM, IPPROTO_UDP);
    if (sock_fd < 0) {
        perror("socket() failed");
        exit(1);
    }

    int opt = 1;
    setsockopt(sock_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    sockaddr_in server_addr;
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(myUDPConfig::SERVER_PORT);

    if (bind(sock_fd, (sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        perror("bind() failed");
        close(sock_fd);
        exit(1);
    }
    printf("[udp_server] bound to port %d, waiting for packets...\n", myUDPConfig::SERVER_PORT);

    // STEP 3: Receive loop. recvfrom() blocks until a packet arrives,
    // and fills in client_addr with WHERE it came from - this is the
    // key difference from TCP's read(): every call tells you the sender.
    char buffer[myUDPConfig::BUFFER_SIZE];
    sockaddr_in client_addr;
    socklen_t client_len = sizeof(client_addr);

    // Track a simple sequence counter to demonstrate a real UDP problem:
    // packets can arrive out of order or go missing entirely, and
    // there's nothing at the socket layer that tells you that happened.
    // (This mirrors, in a tiny way, why protocols like RTP add their
    // own sequence numbers on top of UDP.)
    long last_seq_seen = -1;

    while (true) {
        memset(buffer, 0, myUDPConfig::BUFFER_SIZE);
        ssize_t bytes_received = recvfrom(
            sock_fd, buffer, myUDPConfig::BUFFER_SIZE - 1, 0,
            (sockaddr*)&client_addr, &client_len
        );

        if (bytes_received < 0) {
            perror("recvfrom() failed");
            continue; // one bad packet shouldn't kill the server
        }

        char client_ip[INET_ADDRSTRLEN];
        inet_ntop(AF_INET, &client_addr.sin_addr, client_ip, sizeof(client_ip));

        printf("[udp_server] received %zd bytes from %s:%d -> %s\n",
               bytes_received, client_ip, ntohs(client_addr.sin_port), buffer);

        // Very naive sequence check, assuming the client sends
        // "<seq>:<message>" (see udp_client.cpp).
        long seq = -1;
        char message[myUDPConfig::BUFFER_SIZE];
        if (sscanf(buffer, "%ld:%[^\n]", &seq, message) == 2) {
            if (last_seq_seen != -1 && seq != last_seq_seen + 1) {
                printf("[udp_server] *** gap detected: expected seq %ld, got %ld ***\n",
                       last_seq_seen + 1, seq);
            }
            last_seq_seen = seq;
        }

        // sendto() replies directly to whoever just sent us a
        // packet - no persistent connection needed, we just reuse the
        // address we got from recvfrom().
        ssize_t bytes_sent = sendto(
            sock_fd, buffer, bytes_received, 0,
            (sockaddr*)&client_addr, client_len
        );
        if (bytes_sent < 0) {
            perror("sendto() failed");
        }
    }

    close(sock_fd); // unreachable for now
    return 0;
}