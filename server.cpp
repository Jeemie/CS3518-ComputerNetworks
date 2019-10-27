

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <afxres.h>
#define PORT 80

#ifdef __WIN32__
# include <winsock2.h>
#else
# include <sys/socket.h>
#endif

void err_sys(const char* err) {
    perror(err);
    exit(EXIT_FAILURE);
}

int main(int argc, char const *argv[]) {
    struct sockaddr_in address;
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
    if ((bind (sd, (struct sockaddr*) &address, (socklen_t*)&address))<0) {
        err_sys("bind call error");
    }

    //listen
    if (listen(sd,3)<0) {
        err_sys("listen failure");
    }

    //accept
    if ((new_sd = accept(sd, (struct sockaddr*)&address,(socklen_t*)&address))<0) {
        err_sys("accept failure");
    }

    //read
    read_val = read(new_sd,buf,1024);
    printf("%s\n",buf);

    //write
    send(new_sd,msg,strlen(msg),0);

    //close
    close(new_sd);
}