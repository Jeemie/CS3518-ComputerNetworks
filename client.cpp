//
// Created by Matthew on 10/27/2019.
//
#include <stdio.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <string.h>
#define PORT 80

void err_sys(const char* err) {
    printf("\n");
    printf(err);
    printf("\n");
    return -1;
}

int main(int argc, char const *argv[]) {
    int sd, read_val;
    struct sockaddr_in address;
    address.sin_family = AF_INET;
    address.sin_port = htons(PORT);
    char *msg = "hey server";
    int bufSize = 1024;
    char buf[bufSize] = {0};
    if((sd = socket(AF_INET, SOCK_STREAM, 0 ))<0) {
        err_sys("Socket failure");
    }
    if(inet_pton(AF_INET,"127.0.0.1", &address.sin_addr)<=0) {
        err_sys("Invalid Address")
    }
    if(connect(sd, (struct sockaddr *)&address, sizeof(address))<0) {
        err_sys("Connect failed");
    }
    send(sd, msg, strlen(msg));
    printf("sent hey");
    read_val = read(sd, buf, 1024);
    printf("%s\n",buf);
    close(sd);
}
