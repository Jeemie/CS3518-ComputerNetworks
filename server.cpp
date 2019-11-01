

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <netinet/in.h>
#include <arpa/inet.h>
//#include <fstream>
#include <sstream>
#include <iomanip> //put_time
#include <ctime>
#define PORT 1234
#include <sys/socket.h>

using namespace std;

ostringstream getTime(){

    time_t curr_time = time(NULL);
    struct tm* temp = localtime(&curr_time);
    ostringstream ss;
    ss << put_time(temp, "%Y-%m-%d %H:%M:%S: ");

    return ss;
}

void writeToLog(const char *text){

    FILE* log = fopen("log.txt", "a");

    //printf("%s\n", text);

    ostringstream ss;

    ss << text << std::endl;

    fprintf(log, "%s", ss.str().c_str());
    fflush(log);

}

void err_sys(const char* err) {
    perror(err);
    exit(EXIT_FAILURE);
}

int recImage(int socket){

    char imgBuf[12001];
    FILE* image;
    int size, res;

    //printf("Reading size!\n");

    read(socket, &size, sizeof(int));

    //printf("Size = %i\n", size);

    printf("Reading image array\n");
    char* curr = imgBuf;

    int buf = read(socket, curr, size);

    printf("File read\n");

    printf("Converting!!\n");

    image = fopen("temp.png", "w");
    fwrite(imgBuf, 1, sizeof(imgBuf), image);
    fclose(image);

    system(" java -cp javase.jar:core.jar com.google.zxing.client.j2se.CommandLineRunner temp.png");

    return 0;
}

int main(int argc, char const *argv[]) {

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
    ss << "Server started!";
    writeToLog(ss.str().c_str());
    //accept
    socklen_t clilen = sizeof(clientAddr);

    while(true){
        if ((new_sd = accept(master_socket, (struct sockaddr*)&clientAddr, &clilen))<0) {
            err_sys("accept failure");
        }

        ostringstream ss = getTime();
        //ss << put_time(getTime(), "%Y-%m-%d %H:%M:%S ")
        ss << inet_ntoa(clientAddr.sin_addr) << ": Client connected to server!\n";

        writeToLog(ss.str().c_str());

        //read
        read_val = read(new_sd,buf,1024);
        //printf("%s\n",buf);



        if(recImage(new_sd) == 0){//success


            //write
            send(new_sd,msg,strlen(msg),0);
            ss = getTime();
            ss << inet_ntoa(clientAddr.sin_addr) << "Successfully sent QR Code and decoded by server\n";
            writeToLog(ss.str().c_str());
        }
        //close

        close(new_sd);

        printf("Connection closed! IP = %s, PORT = %i\n", inet_ntoa(clientAddr.sin_addr), ntohs(clientAddr.sin_port));
    }
}



