//
// Created by matt on 11/25/19.
//
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>


struct udp_header{

    u_int32_t src;
    u_int32_t dest;
    u_int16_t packet_length;
    u_int8_t checksum;

};

struct ip_header {
    u_int16_t iph_ihl;
    u_int16_t iph_ver;
    u_int16_t iph_tos = 0;
    u_int16_t iph_len;
    u_int16_t iph_ident;
    u_int16_t iph_flag = 0;
    u_int16_t iph_offset = 0;
    u_int16_t iph_ttl;
    u_int16_t iph_protocol = 17;
    u_int16_t iph_checksum = 0;
    u_int32_t iph_src;
    u_int32_t iph_dest;
};

struct datagram{

    udp_header udph;
    ip_header iph;
    char data[1000];

};
int main() {
    int sockfd;

    char buffer[4096];
    struct ip_header *ipH = (struct ip_header *) buffer;
    struct datagram *datag = (struct datagram*)malloc(sizeof(struct datagram));
    struct udp_header;
    char *msg = "hi im client\n";
    struct sockaddr_in server_addr, client_addr;
    if((sockfd = socket(AF_INET, SOCK_DGRAM, 0))<0) {
        perror("socket call failed");
        exit(EXIT_FAILURE);
    }

    memset(&server_addr,0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(34567);
    server_addr.sin_addr.s_addr = inet_addr("10.0.2.15");

    int buff;
    socklen_t length = sizeof(server_addr);

    sendto(sockfd, (const char*)msg, strlen(msg), MSG_CONFIRM, (const struct sockaddr *) &server_addr, sizeof(server_addr));
    printf("sent\n");
    buff = recvfrom(sockfd, datag, sizeof(struct datagram) , MSG_WAITALL, (struct sockaddr *) &server_addr, &length);
    //buff = recvfrom(sockfd, ipH, sizeof(*ipH), MSG_WAITALL, (struct sockaddr *) &server_addr, &length);

    buffer[buff] = '\0';
    //printf("%i Hi\n", ipH->iph_ttl);
    char str[INET_ADDRSTRLEN];
    //inet_ntop(AF_INET, &ipH->iph_dest, str, INET_ADDRSTRLEN);
    //inet_ntop(AF_INET, buffer.ipH->iph_dest, str, INET_ADDRSTRLEN);
    printf("%i Hi\n", datag->iph.iph_ttl);
    close(sockfd);
    return 0;

}
