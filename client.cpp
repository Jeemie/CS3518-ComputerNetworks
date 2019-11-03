//
// Created by Matthew on 10/27/2019.
//
#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <string.h>
#include <fstream>
#define PORT 1337

using namespace std;
void err_sys(const char* err) {
    printf("\n");
    printf(err);
    printf("\n");
}

int sendImage(int socket){
    char send_buffer[12000], read_buffer[256];
    FILE* QR_PIC;
    QR_PIC = fopen("course_qr.png", "r");

    if(QR_PIC == NULL){
        err_sys("Oopsie it's nullies");
    }
    fseek(QR_PIC, 0, SEEK_END);

    int size = ftell(QR_PIC);
    fseek(QR_PIC, 0, SEEK_SET);
    printf("Sending Image\n");
    //printf("%i\n", size);
    //sprintf(send_buffer, "%.4s", to_string(size).c_str());
    write(socket, &size, sizeof(size));


    //int buf = fread(send_buffer, 1, sizeof(send_buffer), QR_PIC);
    while(!feof(QR_PIC)){
        fread(send_buffer, 1, sizeof(send_buffer), QR_PIC);
        write(socket, send_buffer, sizeof(send_buffer));
        //buf = fread(send_buffer, 1, sizeof(send_buffer), QR_PIC);
    }

    fclose(QR_PIC);
    //send(socket, QR_PIC, sizeof(QR_PIC), 0);
    printf("Finished sending\n");
    return 0;
}

int main(int argc, char const *argv[]) {
    int sd, read_val;
    struct sockaddr_in address;
    address.sin_family = AF_INET;
    address.sin_port = htons(PORT);
    const char *msg = "hey server";
    int bufSize = 1024;
    char buf[bufSize] = {0};
    if((sd = socket(AF_INET, SOCK_STREAM, 0 ))<0) {
        err_sys("Socket failure");
    }
    if(inet_pton(AF_INET,"127.0.0.1", &address.sin_addr)<=0) {
        err_sys("Invalid Address");
    }
    if(connect(sd, (struct sockaddr *)&address, sizeof(address))<0) {
        err_sys("Connect failed");
    }
    send(sd, msg, strlen(msg), 0);
    printf("sent hey\n");
    sendImage(sd);
    read_val = read(sd, buf, 1024);
    printf("Msg = %s\n", buf);
    close(sd);
}