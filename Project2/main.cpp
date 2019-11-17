#include <iostream>
#include <pcap.h>
#include <net/ethernet.h>
#include <netinet/if_ether.h>
#include <netinet/ip.h>
#include <netinet/ether.h>
#include <netinet/in.h>
#include <netinet/udp.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>

using namespace std;

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
    const struct ether_arp* arpH;
    const struct tcphdr* tcpH;
    const struct udphdr* udpH;
    uint16_t arpop;
    char src[INET_ADDRSTRLEN], dst[INET_ADDRSTRLEN];
    int len;
    string output = "";

    size_t headerSize;

    //read the packet into the ethernet header
    eH = (struct ether_header*) packetPointer;
    headerSize = sizeof(struct ether_header);

    //check if the packet is IP
    if(ntohs(eH->ether_type) == ETHERTYPE_IP) {
        cout << "_______________________________________________________________________________________________"<<endl;
        cout << "Ethernet Header:" << endl;
        //cout << "\tMAC Address: " << endl;
        printf("\tTime = %s",ctime((const time_t*)&header->ts.tv_sec));
        printf("\tDest = %s\n", ether_ntoa((const struct ether_addr *) &eH->ether_dhost));
        printf("\tSource = %s\n", ether_ntoa((const struct ether_addr *) &eH->ether_shost));
        //read data into ipHeader, pointer offset by the ethernet header
        ipH = (struct ip*)(packetPointer + headerSize);

        //convert binary addresses into string
        inet_ntop(AF_INET, &(ipH->ip_src), src, INET_ADDRSTRLEN);
        inet_ntop(AF_INET, &(ipH->ip_dst), dst, INET_ADDRSTRLEN);

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

            printf("%s,",ctime((const time_t*)&header->ts.tv_sec));
            //print data
            //for (int i = 0; i < len; i++) {cout << (char) data[i];}
            cout << "_______________________________________________________________________________________________"<<endl;
            cout << "Source: " << src << ":" << srcPort << endl;
            cout << "Destination: " << dst << ":" << dstPort << endl;
            cout << "_______________________________________________________________________________________________"<<endl;


            //cout << output;
            //print ports
            //cout << "Source Port: " << srcPort << endl;
            //cout << "Destination Port: " << dstPort << endl;
        } else if (ipH->ip_p == IPPROTO_UDP){
            headerSize += sizeof(struct ip);
            udpH = (udphdr *) (packetPointer + headerSize);
            srcPort = ntohs(udpH->source);
            dstPort = ntohs(udpH->dest);
            cout << "_______________________________________________________________________________________________"<<endl;
            cout << "UDP Header:" << endl;
            cout << "\tSource: " << src << ":" << srcPort << endl;
            cout << "\tDestination: " << dst << ":" << dstPort << endl;
            cout << "_______________________________________________________________________________________________"<<endl;


            //read in data, offset by ethernet, ip, and tcp header
            headerSize += sizeof(struct udphdr);
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

        }
    }  else if (ntohs(eH->ether_type) == ETHERTYPE_ARP){
        struct in_addr *arpspa;
        struct in_addr *arptpa;
        struct ether_addr *arpsha;
        struct ether_addr *arptha;
        arpH = (struct ether_arp*)(packetPointer + headerSize);
        arpop = ntohs(arpH->arp_op);
        if(arpop == ARPOP_REQUEST) {
            arpspa = (struct in_addr*) arpH->arp_spa;
            arptpa = (struct in_addr*) arpH->arp_tpa;
            arpsha = (struct ether_addr*) arpH->arp_sha;
            arptha = (struct ether_addr*) arpH->arp_tha;
            cout << "_______________________________________________________________________________________________"<<endl;
            cout << "ARP Header:" << endl;
            printf("Sender IP: %s\n", inet_ntoa(*arpspa));
            printf("Sender MAC = %s\n", ether_ntoa(arpsha));
            printf("Target IP: %s\n", inet_ntoa(*arptpa));
            printf("Target MAC = %s\n", ether_ntoa(arptha));
            cout << "_______________________________________________________________________________________________"<<endl;
            cout << "_______________________________________________________________________________________________"<<endl;

        }
    }
}

int main() {
    pcap_t *pcapture;
    char ebuf[PCAP_ERRBUF_SIZE];

    pcapture = pcap_open_offline("cap3.pcap", ebuf);

    if(pcapture == NULL) {
        cout << "read in failed" << endl;
    }

    if(pcap_loop(pcapture, 0, handler, NULL) < 0) {
        cout << "handler read failed" << endl;
        return 1;
    }
}