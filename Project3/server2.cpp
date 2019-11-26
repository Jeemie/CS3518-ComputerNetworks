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
#include <netinet/ip.h>
#include <netinet/udp.h>
#include <netinet/in.h>
struct udp_header{

    u_int32_t src;
    u_int32_t dest;
    u_int16_t packet_length;
    u_int8_t checksum;

};

struct ip_header {
    unsigned char iph_ihl;
    unsigned char iph_ver;
    unsigned char iph_tos;
    u_int16_t iph_len;
    u_int16_t iph_ident;
    unsigned char iph_flag;
    u_int16_t iph_offset;
    unsigned char iph_ttl;
    unsigned char iph_protocol;
    u_int16_t iph_checksum;
    u_int32_t iph_src;
    u_int32_t iph_dest;
};
int main() {
    int sockfd;
    char buffer[8192], *data;
    struct ip_header *iph = (struct ip_header *) buffer;
    struct udp_header *udph = (struct udp_header *) (buffer + sizeof(struct ip_header));



    const char *msg = "hello im the server\n";
    struct sockaddr_in server_addr, client_addr;

    if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0))<0) {
        perror("socket call failed");
        exit(EXIT_FAILURE);
    }

    memset(buffer, 0 , 8192);
    memset(&server_addr, 0, sizeof(server_addr));
    memset(&client_addr, 0, sizeof(client_addr));

    server_addr.sin_family    = AF_INET; // IPv4
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(34567);


    data = buffer + sizeof(struct ip_header) + sizeof(struct udp_header);
    strcpy(data, "ABCDEFGHIJKLMNOPQRSTUVWXYZ");

    iph->iph_ihl = 5;
    iph->iph_ver = 4;
    iph->iph_tos = 0;
    iph->iph_len = sizeof(struct ip_header) + sizeof(struct udp_header) + strlen(data);
    iph->iph_ident = htons(54321);
    iph->iph_ttl = 128;
    iph->iph_protocol = IPPROTO_UDP;
    iph->iph_src = server_addr.sin_addr.s_addr;
    iph->iph_dest = htons(atoi("192.69.69.69"));

    udph->src = htons(8080);
    udph->dest = htons(8080);

    if(bind(sockfd, (const struct sockaddr *)&server_addr, sizeof(server_addr)) <0) {
        perror("bind call failed\n");
    }

    int one = 1;
    const int *val = &one;
    //if(setsockopt(sockfd, IPPROTO_IP, IP_HDRINCL, val, sizeof(one))<0){printf("Warning: HDRINCL not set\n");}

    int buff;
    socklen_t length;
    buff = recvfrom(sockfd, (char*) buffer, 1024, MSG_WAITALL, (struct sockaddr *) &client_addr, &length);
    buffer[buff] = '\0';
    printf("%s\n", buffer);
//    sendto(sockfd, buffer, iph->iph_len, MSG_CONFIRM, (const struct sockaddr *) &client_addr, length);
    sendto(sockfd, (const char*)msg, strlen(msg), MSG_CONFIRM, (const struct sockaddr *) &client_addr, length);
    printf("sent\n");
    return 0;
}