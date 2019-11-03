#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <iostream>
#include <sstream>
#include <fstream>
#include <iomanip> //put_time
#include <ctime>
#include <cstdio>
#include <filesystem>
#include <regex>
#define PORT 1337
#include <sys/socket.h>

using namespace std;

struct Meep {int returnCode; string msg;};


ostringstream getTime(){
    time_t curr_time = time(NULL);
    struct tm* temp = localtime(&curr_time);
    ostringstream ss;
    ss << put_time(temp, "%Y-%m-%d %H:%M:%S: ");

    return ss;
}

void writeToLog(const char *text){

    FILE* log = fopen("log.txt", "a");

    printf("%s\n", text);

    ostringstream ss;

    ss << text << std::endl;

    fprintf(log, "%s", ss.str().c_str());
    fflush(log);

}

void err_sys(const char* err) {
    perror(err);
    exit(EXIT_FAILURE);
}

struct Meep recImage(int socket){
    Meep newMeep;
    FILE* image;

    FILE* tempF = std::tmpfile();
    int size, res;

    printf("Reading size!");
    read(socket, &size, sizeof(int));
    char imgBuf[size];
    //printf("Size = %i\n", size);

    printf("Reading image array\n");
    char* curr = imgBuf;

    int buf = read(socket, curr, size);

    if(buf < 0){
        err_sys("Error reading file");
    }

    printf("File read\n");
    printf("Converting!!\n");
    image = fopen("temp.png", "w");

    if(image == NULL){
        err_sys("Error opening converted file!");
    }


    fwrite(imgBuf, 1, sizeof(imgBuf), image);
    fclose(image);
    ostringstream ss;
    const char* fileName = std::to_string(fileno(tempF)).c_str();
    //printf("NAME= %s\n", fileName);
    ss << "java -cp javase.jar:core.jar com.google.zxing.client.j2se.CommandLineRunner temp.png > ";
    ss <<  fileName;
    //system("java -cp javase.jar:core.jar com.google.zxing.client.j2se.CommandLineRunner temp.png > lol.txt");
    const char* cmd = strdup(ss.str().c_str());
    system(cmd);
    fclose(image);


    string STRING = "No matches found";
    ifstream infile;
    infile.open(fileName);
    regex rgx("d result:\n(https:.*)\n");
    smatch matches;

    ss << infile.rdbuf();

    std::string tempStr = ss.str();

    if(regex_search(tempStr, matches, rgx)) {
        cout << "Match found:\n";
        cout << matches[1].str() << '\n';
        newMeep.msg = matches[1].str();
    }else {
        newMeep.msg = STRING;
    }

    remove(fileName);
    newMeep.returnCode = 0;

    return newMeep;
}

int main(int argc, char const *argv[]) {

    int counter;

    if(argc > 1){
        for(counter = 1; counter < argc; counter++;){

        std::string curr_cmd = argv[counter];
        std::string curr_cmd_name;
        std::string curr_cmd_arg;

            switch(curr_cmd_name){



            }

        }
    }
    struct sockaddr_in address, clientAddr;
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);
    char buf[1024] = {0};
    const char *msg = "hi client";
    int sd, new_sd, read_val, master_socket, client_socket[10], max_clients = 10, activity;
    int opt = 1;

    fd_set readfds;

    int counter;
    for(counter = 0; counter < max_clients; counter++){ client_socket[counter] = 0; }
    // socket
//    if ((sd=socket(AF_INET,SOCK_STREAM,IPPROTO_TCP))<0) {
//        err_sys("socket call failed");
//    }

    if ((master_socket=socket(AF_INET,SOCK_STREAM,0))==0) {
        err_sys("socket call failed");
    }

    if (setsockopt(master_socket, SOL_SOCKET, SO_REUSEADDR, (char*)&opt, sizeof(opt))<0) {
        err_sys("opt failure");
    }

    // bind
    if ((bind (master_socket, (struct sockaddr*)&address, sizeof(address)))<0) {
        err_sys("bind call error");
    }

    //listen
    if (listen(master_socket,3)<0) {
        err_sys("listen failure");
    }
    ostringstream ss = getTime();
    ss << "Server started!\n";
    writeToLog(ss.str().c_str());

    socklen_t clilen = sizeof(clientAddr);

    while(true){
        if ((new_sd = accept(master_socket, (struct sockaddr*)&clientAddr, &clilen))<0) {
            err_sys("accept failure");
        }

        //FORK HERE
        //accept up here and loop
        static int childCount = 0;
        static int counter = 0;
        int pid;
        if ((pid=fork())<0) {
            //unsuccessful creation of child process
            close(new_sd);
            continue;
        } else if(pid>0) {
            //parent
            close(new_sd);
            childCount++;
            counter++;
            printf("made a child: %i\n",childCount);
            continue;
        } else if(pid==0){
            //child process
            //MAX_SIZE
            //receive buffer size inside child process
            //create buffer inside child process
            printf("I'm a child\n");
            counter++;

            //start work
            ss = getTime();
            ss << inet_ntoa(clientAddr.sin_addr) << ": Client connected to server!\n";

            //writeToLog(ss.str().c_str());

            //read
            read_val = read(new_sd,buf,1024);
            //printf("%s\n",buf);
            Meep tempMeep = recImage(new_sd);
            if(tempMeep.returnCode == 0){//success
                send(new_sd,tempMeep.msg.c_str(),strlen(tempMeep.msg.c_str()),0);
                printf("Temp meep!! %s \n", tempMeep.msg.c_str());
                ss = getTime();
                ss << inet_ntoa(clientAddr.sin_addr) << ": Successfully sent QR Code and decoded by server\n";
                writeToLog(ss.str().c_str());
            }
            //close

            close(new_sd);
            printf("Connection closed! IP = %s, PORT = %i\n", inet_ntoa(clientAddr.sin_addr), ntohs(clientAddr.sin_port));
            exit(0);
            //break;
        }
    }
}



