
#include <iostream>         // For standard I/O operations
#include <cstring>          // For memset
#include <cstdlib>          // For exit()
#include <sys/socket.h>     // For socket(), setsockopt(), sendto(), recvfrom()
#include <netinet/ip.h>     // For struct iphdr
#include <netinet/tcp.h>    // For struct tcphdr
#include <arpa/inet.h>      // For inet_addr(), htons()
#include <unistd.h>         // For close()

#define SERVER_PORT 12345   // The port server listens on
#define CLIENT_PORT 54321   // Arbitrary source port for client

// Utility function to print key TCP flags and sequence number
void print_tcp_flags(struct tcphdr *tcp) {
    std::cout << "[+] TCP Flags: "
              << " SYN: " << tcp->syn
              << " ACK: " << tcp->ack
              << " FIN: " << tcp->fin
              << " RST: " << tcp->rst
              << " PSH: " << tcp->psh
              << " SEQ: " << ntohl(tcp->seq) << std::endl;
}

// Function to manually construct and send a TCP packet
void send_packet(int sock, struct sockaddr_in *dest, int seq, int ack, bool syn_flag, bool ack_flag, const std::string &msg) {
    char packet[sizeof(struct iphdr) + sizeof(struct tcphdr)];
    memset(packet, 0, sizeof(packet));

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

    struct tcphdr *tcp = (struct tcphdr *)(packet + sizeof(struct iphdr));
    tcp->source = htons(CLIENT_PORT);
    tcp->dest = htons(SERVER_PORT);
    tcp->seq = htonl(seq);
    tcp->ack_seq = htonl(ack);
    tcp->doff = 5;
    tcp->syn = syn_flag;
    tcp->ack = ack_flag;
    tcp->window = htons(8192);
    tcp->check = 0;

    std::cout << msg << std::endl;
    print_tcp_flags(tcp);

    if (sendto(sock, packet, sizeof(packet), 0, (struct sockaddr *)dest, sizeof(*dest)) < 0) {
        perror("sendto() failed");
        exit(EXIT_FAILURE);
    }
}

int main() {
    int sock = socket(AF_INET, SOCK_RAW, IPPROTO_TCP);
    if (sock < 0) {
        perror("Socket creation failed");
        exit(EXIT_FAILURE);
    }

    int one = 1;
    if (setsockopt(sock, IPPROTO_IP, IP_HDRINCL, &one, sizeof(one)) < 0) {
        perror("setsockopt() failed");
        exit(EXIT_FAILURE);
    }

    struct sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(SERVER_PORT);
    server_addr.sin_addr.s_addr = inet_addr("127.0.0.1");

    // Step 1: Send SYN
    std::cout << "[+] Client sending SYN..." << std::endl;
    send_packet(sock, &server_addr, 200, 0, true, false, "[+] SYN sent");

    char buffer[65536];
    struct sockaddr_in recv_addr;
    socklen_t addr_len = sizeof(recv_addr);

    while (true) {
        int data_size = recvfrom(sock, buffer, sizeof(buffer), 0, (struct sockaddr *)&recv_addr, &addr_len);
        if (data_size < 0) {
            perror("recvfrom() failed");
            continue;
        }

        struct iphdr *ip = (struct iphdr *)buffer;
        struct tcphdr *tcp = (struct tcphdr *)(buffer + (ip->ihl * 4));

        if (ntohs(tcp->dest) != CLIENT_PORT)
            continue;

        std::cout << "[+] Waiting for SYN-ACK from 127.0.0.1..." << std::endl;
        print_tcp_flags(tcp);

        if (tcp->syn == 1 && tcp->ack == 1 && ntohl(tcp->ack_seq) == 201 && ntohl(tcp->seq) == 400) {
            std::cout << "[+] Received SYN-ACK from 127.0.0.1" << std::endl;
            std::cout << "[+] Client sending final ACK..." << std::endl;
            send_packet(sock, &server_addr, 600, 401, false, true, "[+] Final ACK sent");
            std::cout << "[+] Handshake complete." << std::endl;
            break;
        }
    }

    close(sock);
    return 0;
}
