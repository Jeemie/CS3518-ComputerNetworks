

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <netinet/in.h>
//#include <afxres.h>
#define PORT 1234

#ifdef __WIN32__
# include <winsock2.h>
#else
# include <sys/socket.h>
#endif

void err_sys(const char* err) {
    perror(err);
    exit(EXIT_FAILURE);
}

int recImage(int socket){

    char imgBuf[12001];
    FILE* image;
    int size, res;

    printf("Reading size!\n");

    read(socket, &size, sizeof(int));

    printf("Size = %i\n", size);

    printf("Reading image array\n");
    char* curr = imgBuf;
    printf("CURR: %s\n", curr);
    int buf = read(socket, curr, size);
    printf("Hullooo: %i\n", buf);
    //int counter = ;
//    while(buf >=0 && counter > 0){
//        curr += buf;
//        printf("%i\n", buf);
//        buf = read(socket, curr, size);
//        counter--;
//    }

    printf("File read\n");

    printf("Converting!!\n");

    image = fopen("temp.png", "w");
    fwrite(imgBuf, 1, sizeof(imgBuf), image);
    fclose(image);

    system(" java -cp javase.jar:core.jar com.google.zxing.client.j2se.CommandLineRunner temp.png");
    //do{
//       if((res = read(socket, imgBuf, sizeof(imgBuf)))<0){
//           err_sys("oof");
//       };
    //}while(res < 0);
    //printf("Received Packet: %i bytes\n", res);


}

int main(int argc, char const *argv[]) {
    struct sockaddr_in address, clientAddr;
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);
    char buf[1024] = {0};
    char *msg = "hi client";
    int sd, new_sd, read_val;
    int opt = 1;

    // socket
    if ((sd=socket(AF_INET,SOCK_STREAM,IPPROTO_TCP))<0) {
        err_sys("socket call failed");
    }

    if (setsockopt(sd, SOL_SOCKET, SO_REUSEADDR, (char*)&opt, sizeof(opt))<0) {
        err_sys("opt failure");
    }

    // bind
    if ((bind (sd, (struct sockaddr*)&address, sizeof(address)))<0) {
        err_sys("bind call error");
    }

    //listen
    if (listen(sd,3)<0) {
        err_sys("listen failure");
    }

    //accept
    socklen_t clilen = sizeof(clientAddr);
    if ((new_sd = accept(sd, (struct sockaddr*)&clientAddr, &clilen))<0) {
        err_sys("accept failure");
    }

    //read
    read_val = read(new_sd,buf,1024);
    printf("%s\n",buf);

    //write
    send(new_sd,msg,strlen(msg),0);
    recImage(new_sd);
    //close
    close(new_sd);
}



