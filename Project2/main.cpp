#include <iostream>
#include <pcap.h>
#include <string>
#include <net/ethernet.h>
#include <netinet/ip.h>
#include <netinet/ether.h>
#include <netinet/in.h>
#include <netinet/udp.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>
#include <unordered_map>

using namespace std;

unordered_map<string, int> ethernetDest;
unordered_map<string, int> ethernetSrc;
unordered_map<string, int> ipDest;
unordered_map<string, int> ipSrc;
unordered_map<u_int, int> udpDest;
unordered_map<u_int, int> udpSrc;

bool isAscii(int testVal) {
    bool isReturn = testVal==10 || testVal==11 || testVal==13;
    bool isChar = testVal>31 && testVal<127;
    return isReturn || isChar;
}

void handler(u_char* user, const struct pcap_pkthdr* header, const u_char* packetPointer) {
    u_int srcPort, dstPort;
    u_char *data;
    const struct ether_header* eH;
    const struct ip* ipH;
    const struct tcphdr* tcpH;
    char src[INET_ADDRSTRLEN], dst[INET_ADDRSTRLEN];
    int len;
    string output = "";

    size_t headerSize;

    //read the packet into the ethernet header
    eH = (struct ether_header*) packetPointer;

    //check if the packet is IP
    if(ntohs(eH->ether_type) == ETHERTYPE_IP) {
        string eDest(ether_ntoa((const struct ether_addr *) &eH->ether_dhost));
        string eSrc(ether_ntoa((const struct ether_addr *) &eH->ether_shost));
        ethernetDest[eDest]++;
        ethernetSrc[eSrc]++;

        //printf("Count: %i\n", ethernetDest[ether_ntoa((const struct ether_addr *) &eH->ether_dhost)]);

        //printf("Dest = %s\n", ether_ntoa((const struct ether_addr *) &eH->ether_dhost));
        //printf("Source = %s\n", ether_ntoa((const struct ether_addr *) &eH->ether_shost));
        //read data into ipHeader, pointer offset by the ethernet header
        headerSize = sizeof(struct ether_header);
        ipH = (struct ip*)(packetPointer + headerSize);

        //convert binary addresses into string

        inet_ntop(AF_INET, &(ipH->ip_src), src, INET_ADDRSTRLEN);
        inet_ntop(AF_INET, &(ipH->ip_dst), dst, INET_ADDRSTRLEN);

        string source(src);
        string dest(dst);
        ipSrc[source]++;
        ipDest[dest]++;

        //check ip header for TCP or UDP
        if (ipH->ip_p == IPPROTO_TCP) {

            //read tcp header in, header offset by both ethernet and IP header
            headerSize += sizeof(struct ip);
            tcpH = (tcphdr *) (packetPointer + headerSize);

            //read in ports, converted to correct endian format
            srcPort = ntohs(tcpH->source);
            dstPort = ntohs(tcpH->dest);

            //read in data, offset by ethernet, ip, and tcp header
            headerSize += sizeof(struct tcphdr);
            data = (u_char *) (packetPointer + headerSize);

            //the length of the data is equal to the length of the packet minus the combined headers
            len = header->len - headerSize;

            for (int i = 0; i < len; i++) {
                if(isAscii(data[i])){
                    output += (char)data[i];
                } else {
                    output += ".";
                }
            }

            //printf("%s,",ctime((const time_t*)&header->ts.tv_sec));
            //print data
            //for (int i = 0; i < len; i++) {cout << (char) data[i];}
            //cout << endl << "_______________________________________________________________________________________________"<<endl;
            //cout << endl << "Source: " << src << ":" << srcPort << endl;
            //cout << "Destination: " << dst << ":" << dstPort << endl;
            //cout << "_______________________________________________________________________________________________"<<endl;


            //cout << output;
            //print ports
            //cout << "Source Port: " << srcPort << endl;
            //cout << "Destination Port: " << dstPort << endl;
        } else if (ipH->ip_p == IPPROTO_UDP){

        }
    }  else if (ntohs(eH->ether_type) == ETHERTYPE_ARP){
        //cout << ntohs(eH->ether_type) << endl;
        cout << "I'm an ARP packet dump :) I'm not implemented yet\n";
    }
}


void printLists(){
    cout << "Ethernet Destinations!\n";
    for(auto elem: ethernetDest){ cout << "[Destination: "<< elem.first << "] " << "[Count: " << elem.second << "]\n";}
    cout << "Ethernet Sources!\n";
    for(auto elem: ethernetSrc){ cout << "[Source: "<< elem.first << "] " << "[Count: " << elem.second << "]\n";}
    cout << "IP Destinations!\n";
    for(auto elem: ipDest){ cout << "[Dest: "<< elem.first << "] " << "[Count: " << elem.second << "]\n";}
    cout << "IP Sources!\n";
    for(auto elem: ipSrc){ cout << "[Source: "<< elem.first << "] " << "[Count: " << elem.second << "]\n";}
}
int main(int argc, char const *argv[]) {
    if(argc > 2 || argc < 2){
        printf("Invalid arguments. Only arg should be file.\n");
        exit(0);
    } else {
        std::string fileName = argv[1];
        pcap_t *pcapture;
        char ebuf[PCAP_ERRBUF_SIZE];

        pcapture = pcap_open_offline(fileName.c_str(), ebuf);

        if (pcapture == NULL) {
            cout << "read in failed" << endl;
            exit(0);
        }

        if(pcap_datalink(pcapture) != DLT_EN10MB){
            cout << "Not captured using Ethernet\n";
            exit(0);
        }

        if (pcap_loop(pcapture, 0, handler, NULL) < 0) {
            cout << "handler read failed" << endl;
            return 1;
        }
        printLists();

    }
}