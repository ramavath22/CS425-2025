#include <iostream>
#include <cstring>
#include <cstdlib>
#include <sys/socket.h>
#include <netinet/ip.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>
#include <unistd.h>

#define SERVER_PORT 12345   // Server's listening port
#define CLIENT_PORT 54321   // Client's source port

// Debugging function to display TCP flags and sequence number
void print_tcp_flags(struct tcphdr *tcp) {
    std::cout << "[+] TCP Flags: "
              << " SYN: " << tcp->syn
              << " ACK: " << tcp->ack
              << " FIN: " << tcp->fin
              << " RST: " << tcp->rst
              << " PSH: " << tcp->psh
              << " SEQ: " << ntohl(tcp->seq) << std::endl;
}

// Construct and send a raw TCP packet
void send_packet(int sock, struct sockaddr_in *dest, int seq, int ack, bool syn_flag, bool ack_flag, const std::string &msg) {
    char packet[sizeof(struct iphdr) + sizeof(struct tcphdr)];
    memset(packet, 0, sizeof(packet));

    // Fill IP header
    struct iphdr *ip = (struct iphdr *)packet;
    ip->ihl = 5;
    ip->version = 4;
    ip->tos = 0;
    ip->tot_len = htons(sizeof(packet));
    ip->id = htons(12345);
    ip->frag_off = 0;
    ip->ttl = 64;
    ip->protocol = IPPROTO_TCP;
    ip->saddr = inet_addr("127.0.0.1");
    ip->daddr = dest->sin_addr.s_addr;

    // Fill TCP header
    struct tcphdr *tcp = (struct tcphdr *)(packet + sizeof(struct iphdr));
    tcp->source = htons(CLIENT_PORT);
    tcp->dest = htons(SERVER_PORT);
    tcp->seq = htonl(seq);
    tcp->ack_seq = htonl(ack);
    tcp->doff = 5;  // Header length
    tcp->syn = syn_flag;
    tcp->ack = ack_flag;
    tcp->window = htons(8192);
    tcp->check = 0;

    // TODO: Compute and set TCP checksum here for full correctness

    print_tcp_flags(tcp);
    std::cout << msg << std::endl;

    // Send packet using raw socket
    if (sendto(sock, packet, sizeof(packet), 0, (struct sockaddr *)dest, sizeof(*dest)) < 0) {
        perror("sendto() failed");
        exit(EXIT_FAILURE);
    }
}

int main() {
    // Create raw socket for TCP
    int sock = socket(AF_INET, SOCK_RAW, IPPROTO_TCP);
    if (sock < 0) {
        perror("Socket creation failed");
        exit(EXIT_FAILURE);
    }

    // Enable manual IP header inclusion
    int one = 1;
    if (setsockopt(sock, IPPROTO_IP, IP_HDRINCL, &one, sizeof(one)) < 0) {
        perror("setsockopt() failed");
        exit(EXIT_FAILURE);
    }

    // Server address setup
    struct sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(SERVER_PORT);
    server_addr.sin_addr.s_addr = inet_addr("127.0.0.1");

    // Step 1: Send SYN
    std::cout << "[+] Client sending SYN..." << std::endl;
    send_packet(sock, &server_addr, 200, 0, true, false, "[+] SYN sent");

    // Buffer to receive packets
    char buffer[65536];
    struct sockaddr_in recv_addr;
    socklen_t addr_len = sizeof(recv_addr);

    while (true) {
        // Wait for incoming TCP packet
        int data_size = recvfrom(sock, buffer, sizeof(buffer), 0, (struct sockaddr *)&recv_addr, &addr_len);
        if (data_size < 0) {
            perror("recvfrom() failed");
            continue;
        }

        // Parse IP and TCP headers
        struct iphdr *ip = (struct iphdr *)buffer;
        struct tcphdr *tcp = (struct tcphdr *)(buffer + (ip->ihl * 4));

        // Filter out irrelevant responses
        if (ntohs(tcp->dest) != CLIENT_PORT)
            continue;

        std::cout << "[+] Waiting for SYN-ACK from 127.0.0.1..." << std::endl;
        print_tcp_flags(tcp);

        // Check for SYN-ACK response
        if (tcp->syn == 1 && tcp->ack == 1 &&
            ntohl(tcp->ack_seq) == 201 && ntohl(tcp->seq) == 400) {

            std::cout << "[+] Received SYN-ACK from 127.0.0.1" << std::endl;

            // Step 3: Send final ACK
            std::cout << "[+] Client sending final ACK..." << std::endl;
            send_packet(sock, &server_addr, 600, 401, false, true, "[+] Final ACK sent");

            std::cout << "[+] Handshake complete." << std::endl;
            break;
        }
    }

    close(sock);
    return 0;
}
