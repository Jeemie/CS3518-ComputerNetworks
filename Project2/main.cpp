#include <iostream>
#include <pcap.h>
#include <net/ethernet.h>
#include <netinet/ip.h>
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
    const struct tcphdr* tcpH;
    char src[INET_ADDRSTRLEN], dst[INET_ADDRSTRLEN];
    int len;
    string output = "";

    size_t headerSize;

    //read the packet into the ethernet header
    eH = (struct ether_header*) packetPointer;

    //check if the packet is IP
    if(ntohs(eH->ether_type) == ETHERTYPE_IP) {

        //read data into ipHeader, pointer offset by the ethernet header
        headerSize = sizeof(struct ether_header);
        ipH = (struct ip*)(packetPointer + headerSize);

        //convert binary addresses into string
        inet_ntop(AF_INET, &(ipH->ip_src), src, INET_ADDRSTRLEN);
        inet_ntop(AF_INET, &(ipH->ip_dst), dst, INET_ADDRSTRLEN);

        //check ip header for TCP or UDP
        if (ipH->ip_p == IPPROTO_TCP || ipH->ip_p == IPPROTO_UDP) {

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

            //print data
            //for (int i = 0; i < len; i++) {cout << (char) data[i];}
            cout << endl << "_______________________________________________________________________________________________"<<endl;
            cout << endl << "Source: " << src << ":" << srcPort << endl;
            cout << "Destination: " << dst << ":" << dstPort << endl;
            cout << "_______________________________________________________________________________________________"<<endl;


            cout << output;
            //print ports
            //cout << "Source Port: " << srcPort << endl;
            //cout << "Destination Port: " << dstPort << endl;
        }
    }  else if (ntohs(eH->ether_type) == ETHERTYPE_ARP){
        //cout << ntohs(eH->ether_type) << endl;
        cout << "I'm an ARP packet dump :) I'm not implemented yet\n";
    }
}

int main() {
    pcap_t *pcapture;
    char ebuf[PCAP_ERRBUF_SIZE];

    pcapture = pcap_open_offline("cap.pcap", ebuf);

    if(pcapture == NULL) {
        cout << "read in failed" << endl;
    }

    if(pcap_loop(pcapture, 0, handler, NULL) < 0) {
        cout << "handler read failed" << endl;
        return 1;
    }
}