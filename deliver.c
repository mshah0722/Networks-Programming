#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <ctype.h>
#include <netdb.h> 

#define MAXLEN 4096

int main(int argc, char const *argv[]){
    int port;

    //Obtaining port number from argument
    switch(argc) {
        case 3:
            port = atoi(argv[2]);
            break;
        default:
            fprintf(stderr, "usage: deliver -<server address> -<server port number>\n");
            exit(0);
    }
    char* ipAddress = argv[1];
    char* portPointer = argv[2];
    
    printf("Enter filename to transfer in the format: ftp <filename>\n");
    char ftpInput[50], fileName[50];
    scanf("%s %s", ftpInput, fileName);

    //Checking the input for ftp
    if(strcmp(ftpInput, "ftp")!= 0){
        printf("Invalid Command: %s\n", ftpInput);
        exit(0);
    }

    //Finding the file
    if(access(fileName, F_OK) != 0){
        fprintf(stderr, "File not found\n");
        exit(0);
    }

    int status, socketDescriptor;
    struct addrinfo hints;
    struct addrinfo *serverAddress;
    struct sockaddr_storage serverSocketAddress;     //connector's addr info


    memset(&hints, 0, sizeof hints);

    hints.ai_family = AF_UNSPEC;        //IPv4 or IPv6
    hints.ai_socktype = SOCK_DGRAM;     //Connectionless data transfer of fixed max length datagrams
    hints.ai_protocol = IPPROTO_UDP;    //UDP

    status = getaddrinfo(ipAddress, portPointer, &hints, &serverAddress);

    //Creating the UDP socket
    if ((socketDescriptor = socket(serverAddress->ai_family,serverAddress->ai_socktype, serverAddress->ai_protocol)) == -1) {
        fprintf(stderr, "Error with socket\n");
        exit(1);
    }

    //message variables
    char messageReceived[MAXLEN], *ftpResponse;
    int bytesRecveived;

    //Sending ftp response
    ftpResponse = "ftp";
    sendto(socketDescriptor, ftpResponse, strlen(ftpResponse), 0, serverAddress->ai_addr, serverAddress->ai_addrlen);

    socklen_t  addressLength = sizeof(struct sockaddr_storage);
    bytesRecveived = recvfrom(socketDescriptor, messageReceived, MAXLEN-1 , 0, (struct sockaddr *)&serverSocketAddress, &addressLength);
    messageReceived[bytesRecveived] = '\0';


    if (strcmp(messageReceived, "yes") == 0) {
        printf("A file transfer can start.\n");
    }
    freeaddrinfo(serverAddress);
    close(socketDescriptor);
    return 0;
}
