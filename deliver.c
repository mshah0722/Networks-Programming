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

#define MAXBUFLEN 100

int main(int argc, char const *argv[]){
    
    int port;
    int status;
    int sockfd; //Socket File Descriptor

    struct addrinfo hints; //Used to provide hints concerning the type of socket that the caller supports or wishes to use
    struct addrinfo *res; //Pointer to a struct sockaddr containing the destination port and IP Address
    struct sockaddr_storage serverSockAddr; //Connector's addr info

    //Check for correct arguments
    if(argc == 3) 
		port = atoi(argv[2]);
	
    //Print error message and return
    else{
		fprintf(stderr, "usage: deliver <server address> <server port number>\n");
		return 0;
	}

    //Initialize character pointers to the arguments
    const char* ipAddress = argv[1];
    const char* portPointer = argv[2];
    
    //Recieve input from the user and store the input
    printf("Enter file name to transfer in the format: ftp <file name>\n");
    char ftpInput[50], fileName[50];
    scanf("%s %s", ftpInput, fileName);

    //Verifying the input for ftp
    if(strcmp(ftpInput, "ftp")!= 0){
        printf("Invalid Command: %s\n", ftpInput);
        return 0;
    }

    //Checking if the input file actually exists
    if(access(fileName, F_OK) != 0){
        fprintf(stderr, "File not found\n");
        return 0;
    }

    memset(&hints, 0, sizeof hints);

    hints.ai_family = AF_UNSPEC; //IPv4 or IPv6
    hints.ai_socktype = SOCK_DGRAM; //Connectionless data transfer of fixed max length datagrams
    hints.ai_protocol = IPPROTO_UDP; //UDP

    status = getaddrinfo(ipAddress, portPointer, &hints, &res);

    //Creating the UDP socket
    if ((sockfd = socket(res->ai_family, res->ai_socktype, res->ai_protocol)) == -1) {
        fprintf(stderr, "Error with socket\n");
        return 1;
    }

    //Message variables
    char receivedMessage[MAXBUFLEN]; 
    char *ftpResponse;
    
    int receivedBytes;

    //Sending ftp response
    ftpResponse = "ftp";
    sendto(sockfd, ftpResponse, strlen(ftpResponse), 0, res->ai_addr, res->ai_addrlen);

    socklen_t  addrLen = sizeof(struct sockaddr_storage);

    receivedBytes = recvfrom(sockfd, receivedMessage, MAXBUFLEN-1 , 0, (struct sockaddr *)&serverSockAddr, &addrLen);
    receivedMessage[receivedBytes] = '\0';

    //Check whether the correct message is recieved
    if (strcmp(receivedMessage, "yes") == 0) {
        printf("A file transfer can start.\n");
    }
    
    //Free the address info memory
    freeaddrinfo(res);

    //Close the socket file descriptor
    close(sockfd);
    
    return 0;
}
