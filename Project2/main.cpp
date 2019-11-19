#include <iostream>
#include <pcap.h>
#include <string>
#include <net/ethernet.h>
#include <netinet/if_ether.h>
#include <netinet/ip.h>
#include <netinet/ether.h>
#include <netinet/in.h>
#include <netinet/udp.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>
#include <unordered_map>
#include <vector>
using namespace std;

int packetCount = 0, minSize = 999999, maxSize = 0, total = 0;
unordered_map<string, int> ethDestMap;
unordered_map<string, int> ethSrcMap;
unordered_map<string, int> ipDestMap;
unordered_map<string, int> ipSrcMap;
unordered_map<u_int, int> udpDestMap;
unordered_map<u_int, int> udpSrcMap;
unordered_map<string, int> arpIPDestMap;
unordered_map<string, int> arpIPSrcMap;
unordered_map<string, int> arpMACDestMap;
unordered_map<string, int> arpMACSrcMap;



bool isAscii(int testVal) {
    bool isReturn = testVal==10 || testVal==11 || testVal==13;
    bool isChar = testVal>31 && testVal<127;
    return isReturn || isChar;
}

void handlePacket(int size){

    if(size < minSize){minSize = size;}
    if(size > maxSize){maxSize = size;}
    total += size;
    packetCount++;

}

void calcStats(){
    int avg = 0;

    avg = total/packetCount;

    cout << "Avg Size = " << avg << endl;
    cout << "Min packet size = " << minSize << endl;
    cout << "Max packet size = " << maxSize << endl;

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

        string eDest(ether_ntoa((const struct ether_addr *) &eH->ether_dhost));
        string eSrc(ether_ntoa((const struct ether_addr *) &eH->ether_shost));
        ethDestMap[eDest]++;
        ethSrcMap[eSrc]++;

        //printf("Count: %i\n", ethDestMap[ether_ntoa((const struct ether_addr *) &eH->ether_dhost)]);

        //printf("Dest = %s\n", ether_ntoa((const struct ether_addr *) &eH->ether_dhost));
        //printf("Source = %s\n", ether_ntoa((const struct ether_addr *) &eH->ether_shost));

        //read data into ipHeader, pointer offset by the ethernet header
        ipH = (struct ip*)(packetPointer + headerSize);

        //convert binary addresses into string

        inet_ntop(AF_INET, &(ipH->ip_src), src, INET_ADDRSTRLEN);
        inet_ntop(AF_INET, &(ipH->ip_dst), dst, INET_ADDRSTRLEN);

        string source(src);
        string dest(dst);
        ipSrcMap[source]++;
        ipDestMap[dest]++;

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
            handlePacket(len);
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
            cout << "_______________________________________________________________________________________________"<<endl;
            cout << "TCP Header: " << endl;
            cout << "\tSource: " << src << ":" << srcPort << endl;
            cout << "\tDestination: " << dst << ":" << dstPort << endl;
            cout << "_______________________________________________________________________________________________"<<endl;
            cout << "***********************************************************************************************"<<endl;



            //cout << output;
            //print ports
            //cout << "Source Port: " << srcPort << endl;
            //cout << "Destination Port: " << dstPort << endl;
        } else if (ipH->ip_p == IPPROTO_UDP){
            headerSize += sizeof(struct ip);
            udpH = (udphdr *) (packetPointer + headerSize);
            srcPort = ntohs(udpH->source);
            dstPort = ntohs(udpH->dest);
            udpDestMap[dstPort]++;
            udpSrcMap[srcPort]++;
            cout << "_______________________________________________________________________________________________"<<endl;
            cout << "UDP Header:" << endl;
            cout << "\tSource: " << src << ":" << srcPort << endl;
            cout << "\tDestination: " << dst << ":" << dstPort << endl;
            cout << "_______________________________________________________________________________________________"<<endl;
            cout << "***********************************************************************************************"<<endl;


            //read in data, offset by ethernet, ip, and tcp header
            headerSize += sizeof(struct udphdr);
            data = (u_char *) (packetPointer + headerSize);

            //the length of the data is equal to the length of the packet minus the combined headers
            len = header->len - headerSize;
            handlePacket(len);
            for (int i = 0; i < len; i++) {
                if(isAscii(data[i])){
                    output += (char)data[i];
                } else {
                    output += ".";
                }
            }

        }
    }  else if (ntohs(eH->ether_type) == ETHERTYPE_ARP){

        arpH = (struct ether_arp*)(packetPointer + headerSize);
        arpop = ntohs(arpH->arp_op);
        if(arpop == ARPOP_REQUEST) {
            //arpspa = (struct in_addr*) arpH->arp_spa;
            string arpspa(inet_ntoa(*((struct in_addr*) arpH->arp_spa)));
            string arpsha(ether_ntoa((struct ether_addr*) arpH->arp_sha));
            string arptpa(inet_ntoa(*((struct in_addr*) arpH->arp_tpa)));
            string arptha(ether_ntoa((struct ether_addr*) arpH->arp_tha));

            arpIPSrcMap[arpspa]++;
            arpIPDestMap[arptpa]++;
            arpMACSrcMap[arpsha]++;
            arpMACDestMap[arptha]++;

            cout << "_______________________________________________________________________________________________"<<endl;
            cout << "ARP Header:" << endl;
            cout << "Sender IP = " << arpspa << endl;
            cout << "Sender MAC = " << arpsha << endl;
            cout << "Target IP = " << arptpa << endl;
            cout << "Target MAC = " << arptha << endl;
            cout << "_______________________________________________________________________________________________"<<endl;
            cout << "***********************************************************************************************"<<endl;

        }
    }
}

void printSDMap(const char* type, unordered_map<string, int> Smap, unordered_map<string, int> Dmap) {
    cout << type << " Sources:" << endl;
    for(auto elem: Smap){ cout << "\t[Source: " << elem.first << "] " << "[Count: " << elem.second << "]\n";}
    cout << type << " Destinations:" << endl;
    for(auto elem: Dmap){ cout << "\t[Destination: " << elem.first << "] " << "[Count: " << elem.second << "]\n";}
}

void printLists() {
    printSDMap("Ethernet", ethSrcMap, ethDestMap);
    printSDMap("IP", ipSrcMap, ipDestMap);
    cout << "UDP Port Destinations: \n";
    for(auto elem: udpDestMap){ cout << "\t[Dest: " << elem.first << "] " << "[Count: " << elem.second << "]\n";}
    cout << "UDP Port Sources: \n";
    for(auto elem: udpSrcMap){ cout << "\t[Source: " << elem.first << "] " << "[Count: " << elem.second << "]\n";}
    printSDMap("ARP MAC", arpMACSrcMap, arpMACDestMap);
    printSDMap("ARP IP", arpIPSrcMap, arpIPDestMap);
}


int main(int argc, char const *argv[]) {
    if(argc < 2) {
        printf("Must include capture file as argument");
    }
    if(argc > 2) {
        printf("Too many arguments. Capture file is the only argument.\n");
    }
    if(argc > 2 || argc < 2) {
        exit(0);
    } else {
        std::string fileName = argv[1];
        pcap_t *pcapture;
        char ebuf[PCAP_ERRBUF_SIZE];

        pcapture = pcap_open_offline(fileName.c_str(), ebuf);

        if (pcapture == NULL) {
            cout << "Read in failed" << endl;
            exit(0);
        }

        if(pcap_datalink(pcapture) != DLT_EN10MB){
            cout << "Not captured using Ethernet\n";
            exit(0);
        }

        if (pcap_loop(pcapture, 0, handler, NULL) < 0) {
            cout << "Read handler failed" << endl;
            return 1;
        }
        printLists();
        calcStats();

    }
}