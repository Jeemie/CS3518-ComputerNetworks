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

struct udp_header{

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

unsigned short createCSum(unsigned short *buffer, int words){

    unsigned long sum;
    for(sum=0; words>0; words--){
        sum+= *buffer++;
        sum = ( sum >> 16) + (sum &0xffff);
        sum += (sum >> 16);
        return (unsigned short)(~sum);
    }
}


std::unordered_map<std::string,std::string> routingTable;
int main(int argc, char const *argv[]) {

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
                    printf("Hi?\n");
                }
                if(counter%2!=0){
                    printf("There is a lonely IP address somewhere..how depressing.\n");
                    exit(1);
                }
            } else if(curr_cmd.compare("--HOST") == 0) {
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

    //printf("%s\n%s\n%i\n", routerIp.c_str(), hostIp.c_str(), ttl);
    int sockfd;
    char buffer[4096], *data;
    struct datagram *packet = (struct datagram *)malloc(sizeof(struct datagram));
    //struct ip_header *iph = (struct ip_header*)malloc(sizeof(struct ip_header));
    //struct udp_header *udph = (struct udp_header *)malloc(sizeof(struct udp_header));

    const char *msg = "hello im the server\n";



    if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0))<0) {
        perror("socket call failed");
        exit(EXIT_FAILURE);
    }

    //memset(iph, 0 , sizeof(ip_header));
    memset(buffer, 0 , 4096);
    memset(&server_addr, 0, sizeof(server_addr));
    memset(&client_addr, 0, sizeof(client_addr));

    server_addr.sin_family    = AF_INET; // IPv4
    server_addr.sin_addr.s_addr = inet_addr(routerIp.c_str());
    server_addr.sin_port = htons(34567);


    data = buffer;
    strcpy(data, "HereIsSomeData");

    packet->iph.iph_ihl = 5;
    packet->iph.iph_ver = 4;
    packet->iph.iph_tos = 0;
    packet->iph.iph_len = sizeof(struct datagram);
    packet->iph.iph_ident = htons(54321);
    packet->iph.iph_ttl = ttl;
    packet->iph.iph_src = server_addr.sin_addr.s_addr;
    inet_pton(AF_INET, "192.69.69.69", &packet->iph.iph_dest);

    packet->udph.src = htons(8080);
    packet->udph.dest = htons(8080);

    if(bind(sockfd, (const struct sockaddr *)&server_addr, sizeof(server_addr)) <0) {
        perror("bind call failed\n");
    }


    strcpy(packet->data, "HereIsSomeData");
    //printf("%i\n", packet->iph.iph_ihl);
    //printf("%s\n", packet->data);
    int one = 1;
    const int *val = &one;
    //if(setsockopt(sockfd, IPPROTO_IP, IP_HDRINCL, val, sizeof(one))<0){printf("Warning: HDRINCL not set\n");}

    int buff;

    socklen_t length = sizeof(client_addr);
    buff = recvfrom(sockfd, (char*) buffer, 4096, MSG_WAITALL, (struct sockaddr *) &client_addr, &length);
    buffer[buff] = '\0';
    //sendto(sockfd, iph, sizeof(*iph), MSG_CONFIRM, (struct sockaddr *) &client_addr, length);
    sendto(sockfd, packet, packet->iph.iph_len, MSG_CONFIRM, (struct sockaddr *) &client_addr, length);
    //sendto(sockfd, (const char*)msg, strlen(msg), MSG_CONFIRM, (const struct sockaddr *) &client_addr, length);
    printf("sent\n");
    return 0;
}