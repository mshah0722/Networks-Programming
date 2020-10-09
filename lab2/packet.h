#ifndef PACKET_H
#define PACKET_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <errno.h>
#include <time.h>


struct packet {
    unsigned int total_frag;        // Total number of fragments
    unsigned int frag_no;        // Fragment number of packet
    unsigned int size;          // Size of data in bytes
    char* filename;                     //File that is being transferred
    char fileData[1000];        //Data of the provided file
    struct packet * nextPacket;         //Pointer to the next packet
};

//Prints all of the packet details (members of the packet struct)
void printPacket(struct packet * p);

//Prints all of the packets
void printAllPackets(struct packet * p);

//Creates the linked list of packets based on the file
struct packet * fileConvert(char * filename);

//Frees the memory that was used for the linked list
void freePackets(struct packet * root);

//Converting the packet to a string format for sending
char * compressPacket(struct packet * pack, int * len);

//Converting the packet to its individual elements from string format
struct packet * extractPacket(char * packet_str);

#endif
