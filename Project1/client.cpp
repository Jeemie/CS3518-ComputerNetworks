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
#include <iostream>
#include <cerrno>
#include <cstring>
#include <inttypes.h>
#include <cstdint>

using namespace std;
void err_sys(const char* err) {
    printf("\n");
    printf(err);
    printf("\n");
}

int sendImage(int socket, const char* filename){
    char send_buffer[12000], read_buffer[256];
    int okToGo;
    FILE* QR_PIC;
    uint32_t size = 0;
    //printf("%s\n", filename);
    QR_PIC = fopen(filename, "r");
//QR_PIC = fopen("/home/jimmy/Downloads/course_qr.png", "r");

    if(QR_PIC != NULL){
           fseek(QR_PIC, 0, SEEK_END);

            size = ftell(QR_PIC);
            fseek(QR_PIC, 0, SEEK_SET);
            printf("Sending Image\n");
            //printf("%d\n", size);
            //sprintf(send_buffer, "%.4s", to_string(size).c_str());

            //Sending size to server #1
            write(socket, &size, sizeof(size));

            //0 means go ahead, 1 means stop #2
            read(socket, &okToGo, sizeof(okToGo));
            //printf("%ik\n", okToGo);

            //int sending = 0; //0 means sending, 1 means not sending
            if(okToGo == 1){ //Server said not to go
                printf("Image too big :( \n");
                close(socket);
                exit(0);
            }

            //int buf = fread(send_buffer, 1, sizeof(send_buffer), QR_PIC);
            while(!feof(QR_PIC)){ //write #3
                fread(send_buffer, 1, sizeof(send_buffer), QR_PIC);
                write(socket, send_buffer, sizeof(send_buffer));
                //buf = fread(send_buffer, 1, sizeof(send_buffer), QR_PIC);
            }

            fclose(QR_PIC);
            //send(socket, QR_PIC, sizeof(QR_PIC), 0);
            printf("Finished sending\n");
            return 0;
    } else {
        write(socket, &size, sizeof(size));
        std::cout << std::strerror(errno) << '\n';
        close(socket);
        exit(0);
    }

    return 1;

}

int main(int argc, char const *argv[]) {
    int sd, read_val;
    int PORT = 2012;
    std::string serverAddress = "127.0.0.1";
    bool fileGiven = false;
    std::string fileName;
    uint32_t urlSize;
    int counter;

    if(argc > 1){
            for(counter = 1; counter < argc; counter++){

                size_t curr_cmd_length;

                std::string curr_cmd = argv[counter];
                size_t pos = curr_cmd.find("=");
                //curr_cmd_length =
                std::string curr_cmd_name = curr_cmd.substr(0,pos);
                std::string curr_cmd_arg = curr_cmd.substr(pos+1);

                if(pos!=string::npos){
                    if(curr_cmd_name.compare("--PORT") == 0){
                        PORT = stoi(curr_cmd_arg);
                    }

                    else if(curr_cmd_name.compare("--ADDRESS") == 0){
                        serverAddress = curr_cmd_arg;
                    }
                    else if(curr_cmd_name.compare("--FILE") == 0){
                        printf("%s in file loop\n", &curr_cmd_arg[0]);
                        fileName = curr_cmd_arg;
                        fileGiven = true;
                        //printf("%s in for loop\n", fileName);

                    }
                    else {

                        printf("Option %s not found\n", curr_cmd_name.c_str());
                        printf("Options:\n --PORT=<port number> --ADDRESS<IP address> --FILE=<file URL>");

                    }

                    printf("Name=%s\n", curr_cmd_name.c_str());
                    printf("Arg=%s\n", curr_cmd_arg.c_str());
                }

            }
    }

    if(!fileGiven){
        printf("You need to specify a file!\n");
        exit(0);
    }
    printf("FILENAME = %s\n", fileName.c_str());

    struct sockaddr_in address;
    address.sin_family = AF_INET;
    address.sin_port = htons(PORT);
    const char *msg = "hey server";
    uint32_t bufSize = 1024;
    if((sd = socket(AF_INET, SOCK_STREAM, 0 ))<0) {
        err_sys("Socket failure");
    }
//    if(inet_pton(AF_INET,"127.0.0.1", &address.sin_addr)<=0) {
//        err_sys("Invalid Address");
//    }

    if(inet_pton(AF_INET,serverAddress.c_str(), &address.sin_addr)<=0) {
        err_sys("Invalid Address");
    }
    if(connect(sd, (struct sockaddr *)&address, sizeof(address))<0) {
        err_sys("Connect failed");
    }

    if(sendImage(sd, fileName.c_str()) == 0){

        //#4 size
        read(sd, &urlSize, sizeof(uint32_t));
        //urlSize = ntohl(urlSize);
        char buf[urlSize] = {};
        //#5 send
        send(sd, msg, strlen(msg), 0);
        printf("%d is URL SIZE\n", urlSize);
        read_val = read(sd, buf, urlSize+1);
        buf[urlSize] = '\0';//null termination
        printf("Msg = %s\n", buf);
        close(sd);
    }

}