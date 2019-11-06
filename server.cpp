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
#include <cstdint>
#include <filesystem>
#include <regex>
#include <sys/socket.h>

#define MAX_SIZE 10000

using namespace std;

//TODO: Add required logging for error checking, successful interactions, unseccessful interations
//TODO: parent should log attempts to connect over the MAX_USER limit, so it needs to listen for connections and check whether children disconnect
//TODO: error return codes
//TODO: MAX_SIZE
//TODO: timeout
//TODO: rate limiting
//TODO: concurrent logging (at least add a comment on why its not needed due to line buffering)
//TODO: test with different QR codes
//TODO: check for memory leaks with valgrind
//TODO: defaults for arguments
//TODO: keep track of children PID (return value of fork)
//TODO: use kill(0) and check for error to keep track of number of children

struct Meep {uint32_t returnCode; string msg;};


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

    //FILE* image;

    FILE* tempF = std::tmpfile();
    uint32_t size;

    //TODO: Reject oversized images

    //Reading size from server #1
    printf("Reading size!\n");
    read(socket, &size, sizeof(uint32_t));
    char imgBuf[size];
    printf("Size = %i\n", size);
    int okToGo = 0; //0 means go ahead, 1 means stop

    if(size > MAX_SIZE){
        okToGo = 1;
    }

    if(size == 0){
        printf("Size is 0\n");
        newMeep.msg = "Provided non-existent file\n";
        newMeep.returnCode = 1;
        return newMeep;

    }

    ostringstream ss;

    //Write okToGo #2
    write(socket, &okToGo, sizeof(okToGo));

    if(okToGo == 1){ //Not receiving anything
        newMeep.msg = "File too big\n";
        newMeep.returnCode = 1;
        return newMeep;
    }

    printf("Reading image array\n");
    char* curr = imgBuf;

    //#3
    int buf = read(socket, curr, size);

    if(buf < 0){
        err_sys("Error reading file");
    }

    FILE* image;
    printf("File read\n");
    printf("Converting!!\n");

    image = fopen("temp.png", "w");

    if(image == NULL){
        err_sys("Error opening converted file!");
    }

    fwrite(imgBuf, 1, sizeof(imgBuf), image);
    fclose(image);

    //sleep(10);
    //ostringstream ss;
    const char* fileName = std::to_string(fileno(tempF)).c_str();
    //printf("NAME= %s\n", fileName);
    ss << "java -cp javase.jar:core.jar com.google.zxing.client.j2se.CommandLineRunner temp.png > ";
    ss <<  fileName;
    //system("java -cp javase.jar:core.jar com.google.zxing.client.j2se.CommandLineRunner temp.png > lol.txt");
    const char* cmd = strdup(ss.str().c_str());
    system(cmd);
    //fclose(image);


    string STRING = "No matches found\n";
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
        newMeep.returnCode = 1;
    }

    remove(fileName);
    newMeep.returnCode = 0;

    return newMeep;
}

int main(int argc, char const *argv[]) {

    int counter, sd, new_sd, read_val, master_socket, client_socket, max_users, activity;
    int time_out = 60;
    int rate_msg = 3;
    int rate_time= 60;


    int PORT = 2012; //default
    if(argc > 1){
        for(counter = 1; counter < argc; counter++){

        size_t curr_cmd_length;

        std::string curr_cmd = argv[counter];
        size_t pos = curr_cmd.find("=");
        //curr_cmd_length =
        std::string curr_cmd_name = curr_cmd.substr(0,pos);
        std::string curr_cmd_arg = curr_cmd.substr(pos+1);
						//TODO Function arguments from commandline
            if(pos!=string::npos){
                if(curr_cmd_name.compare("--PORT") == 0){
                    PORT = stoi(curr_cmd_arg);
                }
                else if(curr_cmd_name.compare("--RATE_MSGS") == 0){
                    rate_msg = stoi(curr_cmd_arg);
                }
                else if(curr_cmd_name.compare("--RATE_TIME") == 0){
                    rate_time = stoi(curr_cmd_arg);
                }
                else if(curr_cmd_name.compare("--MAX_USERS") == 0){
                    max_users = stoi(curr_cmd_arg);
                }
                else if(curr_cmd_name.compare("--TIME_OUT") == 0){
                    time_out = stoi(curr_cmd_arg);
                }
                else {

                    printf("Option %s not found\n", curr_cmd_name.c_str());
                    printf("Options:\n --PORT=<port number> --RATE_MSGS=<number msgs> --RATE_TIME=<number seconds> --MAX_USERS=<number of users> --TIME_OUT=<number of seconds>");

                }
                printf("Name=%s\n", curr_cmd_name.c_str());
                printf("Arg=%s\n", curr_cmd_arg.c_str());
            }

        }
    }
    struct sockaddr_in address, clientAddr;
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);
    char buf[1024] = {0};
    const char *msg = "hi client";
    int opt = 1;

    fd_set readfds;

    //for(counter = 0; counter < max_users; counter++){ client_socket[counter] = 0; }


    // socket
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

    double max_rate = 1.0*rate_time/rate_msg;
    double print = 25/5;
    //printf("%f\n", max_rate);
    time_t last_time = 0;
    while(true){

        if ((new_sd = accept(master_socket, (struct sockaddr*)&clientAddr, &clilen))<0) {
            err_sys("accept failure");
        }

        time_t now = time(NULL);

        //printf("%f is difference\n",)
        if(now - max_rate < last_time){
            //Too fast! Must wait
            last_time = now;
            close(new_sd);
            break;
            //Make client input again
        }

        last_time = now;

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
            time_t time_connected = time(NULL);
            writeToLog(ss.str().c_str());

            //read
            //read_val = read(new_sd,buf,1024);
            //printf("%s\n",buf);

            //User timed out
            if(time(NULL) - time_connected > time_out){
                ss = getTime();
                ss << inet_ntoa(clientAddr.sin_addr) << ": Client timed out\n";
                writeToLog(ss.str().c_str());

                close(new_sd);
                printf("Connection closed! IP = %s, PORT = %i\n", inet_ntoa(clientAddr.sin_addr), ntohs(clientAddr.sin_port));
                exit(0);
            }


            Meep tempMeep = recImage(new_sd);
            if(tempMeep.returnCode == 0){//success
                //uint32_t urlsize = htonl((uint32_t)strlen(tempMeep.msg.c_str()));
                uint32_t urlsize = (uint32_t)strlen(tempMeep.msg.c_str());
                printf("URL size = %d\n", (uint32_t)strlen(tempMeep.msg.c_str()));

                //#4 size
                write(new_sd,&urlsize,sizeof(urlsize));
                //#5 send
                read_val = read(new_sd,buf,1024);
                //#6 write
                write(new_sd,tempMeep.msg.c_str(), urlsize);
                printf("Temp meep!! %s \n", tempMeep.msg.c_str());
                ss = getTime();
                ss << inet_ntoa(clientAddr.sin_addr) << ": Successfully sent QR Code and decoded by server\n";
                writeToLog(ss.str().c_str());
            }

            if(tempMeep.returnCode == 1){

                ss = getTime();
                ss << inet_ntoa(clientAddr.sin_addr) << tempMeep.msg;
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



