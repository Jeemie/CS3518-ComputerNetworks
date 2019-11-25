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

int main() {
    int sockfd;
    char buffer[1024];
    const char *msg = "hello im the server\n";
    struct sockaddr_in server_addr, client_addr;

    if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0))<0) {
        perror("socket call failed");
        exit(EXIT_FAILURE);
    }

    memset(&server_addr, 0, sizeof(server_addr));
    memset(&client_addr, 0, sizeof(client_addr));
    server_addr.sin_family    = AF_INET; // IPv4
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(1234);

    if(bind(sockfd, (const struct sockaddr *)&server_addr, sizeof(server_addr)) <0) {
        perror("bind call failed");
    }

    int buff;
    socklen_t length;
    buff = recvfrom(sockfd, (char*) buffer, 1024, MSG_WAITALL, (struct sockaddr *) &client_addr, &length);
    buffer[buff] = '\0';
    printf("%s\n", buffer);
    sendto(sockfd, (const char *)msg, strlen(msg), MSG_CONFIRM, (const struct sockaddr *) &client_addr, length);
    printf("sent\n");
    return 0;
}