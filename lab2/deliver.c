#include "packet.h"

//Max Buffer Length
#define MAXBUFLEN 100
#define MAXFILELEN 1000

int main(int argc, char const *argv[]){
    
    int port;
    int status;
    int sockfd; //Socket File Descriptor

    const char msg_yes[] = "yes";
    const char msg_no[] = "no";
    const char msg_ftp[] = "ftp";

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
    if(strcmp(ftpInput, msg_ftp)!= 0){
        printf("no\n");
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
    //char *ftpResponse;
    
    int receivedBytes;

    //Section 2: Measure the round-trip time from the client to the server.
    clock_t startingTime, endingTime
    startingTime = clock();

    //Sending ftp response
    //ftpResponse = msg_ftp;
    sendto(sockfd, msg_ftp, strlen(msg_ftp), 0, res->ai_addr, res->ai_addrlen);

    socklen_t addrLen = sizeof(struct sockaddr_storage);

    receivedBytes = recvfrom(sockfd, receivedMessage, MAXBUFLEN-1 , 0, (struct sockaddr *)&serverSockAddr, &addrLen);
    receivedMessage[receivedBytes] = '\0';

    //Getting the round trip time after receiving the response from the server
    endingTime = clock()
    float roundTripTime = ((float) (endingTime - startingTime)/CLOCKS_PER_SEC));
    printf("Round-Trip Time: %f seconds\n", roundTripTime);
	
    //Check whether the correct message is recieved
    if (strcmp(receivedMessage, msg_yes) == 0) {
	    printf("%s\n", msg_yes);
        printf("A file transfer can start.\n");
    }

    //Tranfering the file as packets
    int length;

    struct packet* rootPacket = fileConvert(fileName);
    struct packet* currentPacket = rootPacket;
    
    while(currentPacket != NULL) {
        char* compressedPacket = compressPacket(currentPacket, &length);
        
        //Sending the packet
        receivedBytes = sendto(sockfd, compressedPacket, length, 0, serverAddress->ai_addr, serverAddress->ai_addrlen);
        
        //Receiving packets
        receivedBytes = recvfrom(sockfd, receivedMessage, MAXFILELEN - 1, 0, (struct sockaddr *)&serverSockAddr, &addrLen);
        
        //Adding \0 for string comparison
        receivedMessage[receivedBytes] = '\0';
        
        //Checking to see if the packets have been acknowledged
        if (strcmp(receivedMessage, "ACK") != 0)
            continue;

        printf("Packet %d has been sent.\n", currentPacket->frag_no);
        
        //Go to next packet in the the linked list and free the current packet
        currentPacket = currentPacket->nextPacket;
        free(compressedPacket);
    }
    
    //Free the address info memory
    freeaddrinfo(res);

    //Close the socket file descriptor
    close(sockfd);
    
    return 0;
}
