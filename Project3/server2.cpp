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

unordered_map<string, string> routingTable;

int main(int argc, char const *argv[]) {

    string s0 = "f";
    string s1 = "g";
    routingTable.insert(pair<string,string>(s0,s1));
    routingTable.insert(pair<string,string>(string("a"),string("b")));
    routingTable[string("test")]=string("value");

    for( auto str = routingTable.begin(); str != routingTable.end(); ++str) {
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

    cout << test5 << " is " << routingTable.at(test5) << endl;

    //if (router), (IP:oIP), ... -> routingTable.insert
    //router should allow connections from INADDR_ANY
    //once router receives a connection, it should wait for a packet size, followed by the packets
    //packets should contain a overlay address
    //realADDRstr = routingTable.at(overlayIP)
    //create sock_addr* saddr for connection
    //saddr.sin_addr = inet_pton(realADDRstr) -> INET_ADDR
    //bind to saddr, create new overlay packet with overlayIP
    //sendTo(bindsocket, newPkt, sizeof(datagram), 0, (struct sock_addr) &saddr, sizeof(&saddr))

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







    struct sockaddr_in server_addr, client_addr, sa_test_dummy;

    /*
    if(argc == 3){
        std::string curr_cmd = argv[1];

        if(curr_cmd.compare("--HOST") == 0 || curr_cmd.compare("--ROUTER") == 0){
            std::string arg = argv[2];
            std::istringstream ss(arg);
            std::string token;

            int counter = 0;
            while(std::getline(ss, token, ':')) {
                if(inet_pton(AF_INET, token.c_str(), &sa_test_dummy.sin_addr) != 1) {
                    printf("You gave an invalid IP address somewhere :(\n");
                } else {
                    std::cout << token << "\n";
                    counter++;
                }
            }
            if(counter%2!=0){
                printf("There is a lonely IP address somewhere..how depressing.\n");
                exit(1);
            }


        } else {
            printf("Options: --ROUTER <list of host IP mappings, --HOST <router IP>,<host IP>, <TTL>\n");
            printf("Did you make sure the options are capitalized?\n");
            exit(1);
        }

    } else {
        printf("Invalid use of program.\n");
        printf("Usage: ./server --option --args\n");
        printf("Options: --ROUTER <list of host IP mappings, --HOST <router IP>,<host IP>, <TTL>\n");
        exit(1);
    }
*/

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
    server_addr.sin_addr.s_addr = inet_addr("10.0.2.15");
    server_addr.sin_port = htons(34567);


    data = buffer;
    strcpy(data, "HereIsSomeData");

    packet->iph.iph_ihl = 5;
    packet->iph.iph_ver = 4;
    packet->iph.iph_tos = 0;
    packet->iph.iph_len = sizeof(struct datagram);
    packet->iph.iph_ident = htons(54321);
    packet->iph.iph_ttl = 420;
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