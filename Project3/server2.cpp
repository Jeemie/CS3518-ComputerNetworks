//
// Created by matt on 11/25/19.
//
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <string>
#include <regex>
#include <iostream>
#include <sstream>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/ip.h>
#include <netinet/udp.h>
#include <netinet/in.h>
#include <unordered_map>
using namespace std;


struct udp_header {

    u_int32_t src;
    u_int32_t dest;
    u_int16_t packet_length;
    u_int8_t checksum;

};

struct ip_header {
    u_int16_t iph_ihl;
    u_int16_t iph_ver;
    u_int16_t iph_tos = 0;
    u_int16_t iph_len;
    u_int16_t iph_ident;
    u_int16_t iph_flag = 0;
    u_int16_t iph_offset = 0;
    u_int16_t iph_ttl;
    u_int16_t iph_protocol = 17;
    u_int16_t iph_checksum = 0;
    u_int32_t iph_src;
    u_int32_t iph_dest;
};

struct datagram{

    udp_header udph;
    ip_header iph;
    char data[1000];

};


unordered_map<string, string> routingTable;

void createPacket(datagram *packet, in_addr_t dst, in_addr_t src, u_int16_t ttl){

    packet->iph.iph_ihl = 5;
    packet->iph.iph_ver = 4;
    packet->iph.iph_tos = 0;
    packet->iph.iph_len = sizeof(struct datagram);
    packet->iph.iph_ident = htons(54321);
    packet->udph.src = htons(8080);
    packet->udph.dest = htons(8080);
    packet->iph.iph_ttl = ttl;
    packet->iph.iph_src = src;
    packet->iph.iph_dest = dst;
    //inet_pton(AF_INET, "192.69.69.69", &packet->iph.iph_dest);

}

int main(int argc, char const *argv[]) {

/*    string s0 = "f";
    string s1 = "g";
    routingTable.insert(pair<string,string>(s0,s1));
    routingTable.insert(pair<string,string>(string("a"),string("b")));
    routingTable[string("test")]=string("value");*/
    int sockfd, buff;
    char buffer[4096], *data;
    struct datagram *packet = (struct datagram *)malloc(sizeof(struct datagram));

    bool isClient = false;

/*    for( auto str = routingTable.begin(); str != routingTable.end(); ++str) {
        cout << str->first; //key output
        string& value = str-> second;
        cout << ":" << value << endl;
    }
    cout<< "there is " << routingTable.at(s0) << " at " << s0 << endl;

    //receive strings from terminal
    string test = string("8.8.8.8");
    string test2 = string("1.1.1.1");
    cout << "test: " << test << endl;
    cout << "test2: " << test2 << endl;

    //convert strings to IP addresses for testing
    in_addr* testIP;
    inet_pton(AF_INET, test.c_str(), &testIP);

    in_addr* test2IP;
    inet_pton(AF_INET, test2.c_str(), &test2IP);

    //convert back to store into map
    char test3[INET_ADDRSTRLEN];
    char test4[INET_ADDRSTRLEN];
    inet_ntop(AF_INET, &testIP, test3, INET_ADDRSTRLEN);
    inet_ntop(AF_INET, &test2IP, test4, INET_ADDRSTRLEN);
    string stest3 = string(test3);
    string stest4 = string(test4);
    cout << stest3 << endl;
    cout << stest4 << endl;

    //AF_INET, struct, str, INET_ADDRSTRLEN

    //put strings into table
    routingTable.insert(pair<string,string>(stest3,stest4));

    //string to lookup received from packet
    string test5 = "8.8.8.8";

    cout << test5 << " is " << routingTable.at(test5) << endl;*/

    struct sockaddr_in server_addr, client_addr, sa_test_dummy;
    std::string routerIp = "10.0.2.15";
    std::string hostIp;
    int ttl = 0;

    if(argc == 3){
        std::string curr_cmd = argv[1];

        if(curr_cmd.compare("--HOST") == 0 || curr_cmd.compare("--ROUTER") == 0){
            std::string arg = argv[2];
            std::istringstream ss(arg);
            std::string token;

            int counter = 0;
            int key = 0;
            if(curr_cmd.compare("--ROUTER") == 0){
                while(std::getline(ss, token, ',')) {
                    std::stringstream pair(token);
                    std::string key;
                    std::string value;

                    size_t pos = 0;

                    int strLength = token.length();
                    pos = token.find(':');
                    key = token.substr(0, pos);
                    value = token.substr(pos+1, strLength);
                    if (inet_pton(AF_INET, value.c_str(), &sa_test_dummy.sin_addr) != 1 || inet_pton(AF_INET, key.c_str(), &sa_test_dummy.sin_addr) != 1) {
                        printf("You gave an invalid IP address somewhere :(\n");
                        exit(1);
                    } else {
                        routingTable[key] = value;
                        std::cout << key << '\n';
                        std::cout << value << '\n';
                        counter++;
                    }
                }
/*                if(counter%2!=0){
                    printf("There is a lonely IP address somewhere..how depressing.\n");
                    exit(1);
                }*/
            } else if(curr_cmd.compare("--HOST") == 0) {
                isClient = true;
                while(std::getline(ss, token, ',')) {
                    if(inet_pton(AF_INET, token.c_str(), &sa_test_dummy.sin_addr) != 1 && counter !=2) {
                        printf("You gave an invalid IP address somewhere :(\n");
                        exit(1);
                    } else {

                        switch(counter){
                            case 0:
                                routerIp = token;
                                break;
                            case 1:
                                hostIp = token;
                                break;
                            case 2:
                                int test = atoi(token.c_str());

                                if(test > 0){
                                    ttl = test;
                                } else{
                                    printf("Y'all didn't give a positive integer for TTL or had it at 0.\n");
                                    exit(1);
                                }

                                break;
                        }
                        //std::cout << token << "\n";
                        counter++;
                    }
                }

                if(counter !=3){
                    printf("Not enough arguments. --HOST <router IP>,<host IP>,<TTL>\n");
                    exit(1);
                }
            }


        } else {
            printf("Options: --ROUTER <list of host IP mappings, --HOST <router IP>,<host IP>,<TTL>\n");
            printf("Did you make sure the options are capitalized?\n");
            exit(1);
        }

    } else {
        printf("Invalid use of program.\n");
        printf("Usage: ./server --option --args\n");
        printf("Options: --ROUTER <list of host IP mappings, --HOST <router IP>,<host IP>, <TTL>\n");
        exit(1);
    }


    if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0))<0) {
        perror("socket call failed");
        exit(EXIT_FAILURE);
    }


    //I'm host stuff c:
    if(isClient){





        //if (client), (serverIP)
        //wait for pkt size on one thread, read source overlay IP
        //check if dest overlay IP addr inside packet is correct for the client, else drop it
        //check if the src of the received packet was the router, if not, drop it
        //wait for packets and check for correct sizes and continuity and order
        //check if sourceOverlayIP.bin exists, if not, create it
        //append to sourceOverlayIP.bin

        //on other thread:
        //scan for files that fit correct shape (regex?)
        //read in overlay addr from file name
        //read in size of packet, create a packet to send the size
        //send the file in groups of 1000 bytes - any info we need to send
        //



        //memset(iph, 0 , sizeof(ip_header));
        memset(buffer, 0 , 4096);
        memset(packet, 0, sizeof(datagram));
        memset(&server_addr, 0, sizeof(server_addr));
        memset(&client_addr, 0, sizeof(client_addr));

        server_addr.sin_family    = AF_INET; // IPv4


        inet_pton(AF_INET, routerIp.c_str(), &server_addr.sin_addr.s_addr);
        inet_pton(AF_INET, routerIp.c_str(), &client_addr.sin_addr.s_addr);


        //server_addr.sin_addr.s_addr = inet_addr(routerIp.c_str());
        cout << "here: " <<routerIp.c_str() << endl;
        server_addr.sin_port = htons(34567);

/*        if(bind(sockfd, (const struct sockaddr *)&server_addr, sizeof(server_addr)) <0) {
            perror("bind call failed\n");
        }*/




        createPacket(packet, client_addr.sin_addr.s_addr, server_addr.sin_addr.s_addr, ttl);

        char test4[INET_ADDRSTRLEN];
        inet_ntop(AF_INET, &packet->iph.iph_dest, test4, INET_ADDRSTRLEN);
        string stest4 = string(test4);
        cout << stest4 << endl;

        strcpy(packet->data, "HereIsSomeData");

        socklen_t length = sizeof(client_addr);
        //buff = recvfrom(sockfd, (char*) buffer, 4096, MSG_WAITALL, (struct sockaddr *) &client_addr, &length);
        buffer[buff] = '\0';

        /*sendto(sockfd, packet, packet->iph.iph_len, MSG_CONFIRM, (struct sockaddr *) &server_addr, length);
        printf("sent\n");
        buff = recvfrom(sockfd, packet, packet->iph.iph_len, MSG_WAITALL, (struct sockaddr *) &client_addr, &length);
        printf("Received\n");
        printf("%i Hi\n", packet->iph.iph_ttl);

*/

        FILE* bin;
        uint32_t size = 0;
        int okToGo = 1;
        //printf("%s\n", filename);
        bin = fopen("test.bin", "r");

        if(bin != NULL) {
            fseek(bin, 0, SEEK_END);

            size = ftell(bin);
            fseek(bin, 0, SEEK_SET);
            printf("Sending Image\n");

            sendto(sockfd, &size, sizeof(size), MSG_CONFIRM, (struct sockaddr *) &server_addr, length);
            buff = recvfrom(sockfd, &okToGo, sizeof(okToGo), MSG_WAITALL, (struct sockaddr *) &server_addr, &length);

            char send_buffer[1000], read_buffer[1000];
            if(okToGo == 0){
                cout << "It's all good to go :D\n";
                while(!feof(bin)){ //write #3

                    fread(packet->data, 1, sizeof(packet->data), bin);
                    cout << sizeof(packet->data) << "I'm the size of data \n";
                    fread(send_buffer, 1, sizeof(send_buffer), bin);
                    //sendto(sockfd, send_buffer, sizeof(send_buffer), MSG_CONFIRM, (struct sockaddr *) &server_addr, length);
                    sendto(sockfd, packet, sizeof(struct datagram), MSG_CONFIRM, (struct sockaddr *) &server_addr, length);
                    //write(socket, send_buffer, sizeof(send_buffer));
                }

            }
        }

        cout << size << " is the size :D \n";
    } else { //I'm a router :c

        //if (router), (IP:oIP), ... -> routingTable.insert
        //Decrement TTL
        //router should allow connections from INADDR_ANY
        //once router receives a connection, it should wait for a packet size, followed by the packets
        //packets should contain a overlay address
        //realADDRstr = routingTable.at(overlayIP)
        //create sock_addr* saddr for connection
        //saddr.sin_addr = inet_pton(realADDRstr) -> INET_ADDR
        //bind to saddr, create new overlay packet with overlayIP
        //sendTo(bindsocket, newPkt, sizeof(datagram), 0, (struct sock_addr) &saddr, sizeof(&saddr))
        //receive connection from any client

        char name[INET_ADDRSTRLEN];
        const int serverPort = 34568;
        memset(&server_addr, 0, sizeof(server_addr));
        server_addr.sin_family    = AF_INET; // IPv4
        server_addr.sin_addr.s_addr = INADDR_ANY;
        server_addr.sin_port = htons(34567);

        if(bind(sockfd, (const struct sockaddr *)&server_addr, sizeof(server_addr)) <0) {
            perror("bind call failed\n");
        }
        while(true) {






            //read message into buffer
            socklen_t length = sizeof(server_addr);
            //buff = recvfrom(sockfd, (char*) buffer, 4096, MSG_WAITALL, (struct sockaddr *) &server_addr, &length);
            //buff = recvfrom(sockfd, packet, sizeof(struct datagram) , MSG_WAITALL, (struct sockaddr *) &server_addr, &length);
            struct sockaddr_in send_addr;
            memset(&send_addr,0,sizeof(send_addr));

            uint32_t size;
            int okToGo = 1;

            //TODO: Reject oversized images

            //Reading size from server #1
            printf("Reading size!\n");
            //read(sockfd, &size, sizeof(uint32_t));
            buff = recvfrom(sockfd, &size, sizeof(size), MSG_WAITALL, (struct sockaddr *) &server_addr, &length);
            cout << size << "Here is the router size o-o\n";

            if(size > 0){okToGo = 0;}

            sendto(sockfd, &okToGo, sizeof(okToGo), MSG_CONFIRM, (struct sockaddr *) &server_addr, length);

            char imgBuf[size];
            char* curr = imgBuf;

            //#3
            //int buf = read(socket, curr, size);
            //buff = recvfrom(sockfd, curr, size, MSG_WAITALL, (struct sockaddr *) &server_addr, &length);
            buff = recvfrom(sockfd, packet, sizeof(struct datagram), MSG_WAITALL, (struct sockaddr *) &server_addr, &length);
            if(buff < 0){
                cout << "File no read :(\n";
            } else {
                cout << "File read :DD\n";

                FILE* image;

                image = fopen("copy3.bin", "w");

                if(image == NULL) {
                    cout << "Can't open copy file :c\n";
                }
                cout << packet->data << "Ouchie ouch\n";
                fwrite(packet->data, 1, sizeof(imgBuf), image);
                //fwrite(imgBuf, 1 , sizeof(imgBuf), image);
                fclose(image);

                exit(1);
            }

            printf("%i Hi\n", packet->iph.iph_ttl);
            inet_ntop(AF_INET, &packet->iph.iph_dest, name, INET_ADDRSTRLEN);
            //std::cout<<"name: "<<packet->iph.iph_dest <<"\n";
            packet->iph.iph_ttl--;
            if(packet->iph.iph_ttl <= 0){
                printf("Packet dropped. :( TTL = < 0 \n");
            } else {
                send_addr.sin_family = AF_INET;
/*                char test4[INET_ADDRSTRLEN];
                inet_ntop(AF_INET, &packet->iph.iph_dest, test4, INET_ADDRSTRLEN);
                string stest4 = string(test4);
                cout << stest4 << endl;*/

                std::string key = string(name);
                std::string destination = routingTable.at(key);
                inet_pton(AF_INET, destination.c_str(), &send_addr.sin_addr.s_addr);
                send_addr.sin_port = htons(34567);

                std::cout << destination << "Dest\n";
                //send_addr.sin_addr.s_addr = inet_addr(destination.c_str());

                sendto(sockfd, packet, packet->iph.iph_len, MSG_CONFIRM, (struct sockaddr *) &send_addr, length);

            }
            //read from message: TTL, decrement, forward to IP address in packet

        }
    }
    //printf("%s\n%s\n%i\n", routerIp.c_str(), hostIp.c_str(), ttl);



    return 0;
}