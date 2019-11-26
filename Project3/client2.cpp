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
    char *msg = "hi im client\n";
    struct sockaddr_in server_addr;
    if((sockfd = socket(AF_INET, SOCK_DGRAM, 0))<0) {
        perror("socket call failed");
        exit(EXIT_FAILURE);
    }

    memset(&server_addr,0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(34567);
    server_addr.sin_addr.s_addr = inet_addr("10.0.2.15");

    int buff;
    socklen_t length;

    sendto(sockfd, (const char*)msg, strlen(msg), MSG_CONFIRM, (const struct sockaddr *) &server_addr, sizeof(server_addr));
    printf("sent\n");
    buff = recvfrom(sockfd, (char *) buffer, 1024, MSG_WAITALL, (struct sockaddr *) &server_addr, &length);
    buffer[buff] = '\0';
    printf("%s Hi\n", buffer);
    close(sockfd);
    return 0;

}
