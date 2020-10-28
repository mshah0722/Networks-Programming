#include "functions.h"

//Max Buffer Length
#define MAXBUFLEN 1100
#define MAXFRAGLEN 1000

int main(int argc, char const *argv[]){
    
    int port;
    int status;
    int sockfd; //Socket File Descriptor

    const char msg_yes[] = "yes";
    const char msg_no[] = "no";
    const char msg_ftp[] = "ftp";
    const char msg_ACK[] = "ACK";

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
    printf("Use the following format:(ftp <file name>) to transfer a file\n");
    char ftpInput[50], filename[50];
    scanf("%s %s", ftpInput, filename);

    //Verifying the input for ftp
    if(strcmp(ftpInput, msg_ftp)!= 0){
        printf("%s\n", msg_no);
        printf("Unknown Command: %s\n", ftpInput);
        return 0;
    }
    
    //Checking if the input file actually exists
    if(access(filename, F_OK) != 0){
        fprintf(stderr, "File does not exist\n");
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
    
    int receivedBytes;

    //Measure the round-trip time from the client to the server.
    clock_t startingTime, endingTime;
    startingTime = clock();

    //Sending ftp response
    sendto(sockfd, msg_ftp, strlen(msg_ftp), 0, res->ai_addr, res->ai_addrlen);

    socklen_t addrLen = sizeof(struct sockaddr_storage);

    receivedBytes = recvfrom(sockfd, receivedMessage, MAXBUFLEN-1 , 0, (struct sockaddr *)&serverSockAddr, &addrLen);
    
    //Adding null character to the end
    receivedMessage[receivedBytes] = '\0';

    //Getting the round trip time after receiving the response from the server
    endingTime = clock();
    float roundTripTime = ((float) (endingTime - startingTime)/CLOCKS_PER_SEC);
    printf("Round-Trip Time: %f seconds\n", roundTripTime);
	
    //Check whether the correct message is recieved
    if (strcmp(receivedMessage, msg_yes) == 0) {
	    printf("%s\n", msg_yes);
        printf("A file transfer can start.\n");
    }

    //Calculating values for timeout functionality
    const double alpha = 0.125;
    const double beta = 0.25;

    struct timeval timeout;
    
    double sampleRTT, devRTT, timeoutInterval;
    bool flagRetransmission = false;

    sampleRTT = roundTripTime;
    devRTT = roundTripTime;
    timeoutInterval = sampleRTT + 4*devRTT;

    //The File Descriptor is set to recieve packets
    fd_set readfd;
    FD_ZERO(&readfd);

    //Tranfering the file as packets
    int length;

    struct packet* head_packet = create_linked_packets(filename);
    printf("total pacets: %d \n",head_packet->total_frag);
    struct packet* current_packet = head_packet;
    
    //Loop till end of file is reached
    while(current_packet != NULL) {
        
        //Converting packet from struct to string format
        char* final_string = struct_to_string(current_packet, &length);
        
        //Sending the packet
        startingTime = clock();
        receivedBytes = sendto(sockfd, final_string, length, 0, res->ai_addr, res->ai_addrlen);
        
        //Monitoring the socket descriptor and setting the timeout
        timeout.tv_sec = timeoutInterval/1;
        timeout.tv_usec = (timeoutInterval - timeout.tv_sec)*1000000;

        FD_SET(sockfd, &readfd);
        select(sockfd+1, &readfd, NULL, NULL, &timeout);
        
        //Clock timed out  
        //Need to retransmit the packet
        if(!FD_ISSET(sockfd, &readfd)) {
            printf("Error: Timeout Occurred\n");
            
            flagRetransmission = true;
            timeoutInterval *= 2;
            
            free(final_string);
            
            continue;
        }


        //Receiving ACK response from server 
        receivedBytes = recvfrom(sockfd, receivedMessage, MAXFRAGLEN - 1, 0, (struct sockaddr *)&serverSockAddr, &addrLen);
        endingTime = clock();

        //Adding \0 to terminate string for string comparison
        receivedMessage[receivedBytes] = '\0';
        
        //Checking if the packets have been acknowledged
        //Else retransmitting the packet
        if (strcmp(receivedMessage, msg_ACK) != 0){
            free(final_string);
            
            flagRetransmission = true;
            
            continue;
        }
        //Checking if packets are sent
        printf("Packet %d has been sent and acknowledged.\n", current_packet->frag_no);
        
        //Calculate new timeout values if the packet did not need to retransmit
        if (flagRetransmission){
            flagRetransmission = false;
        }

        else {
            roundTripTime = ((double) (endingTime - startingTime)/CLOCKS_PER_SEC);
            sampleRTT = (1-alpha) * sampleRTT + alpha * roundTripTime;
            devRTT = (1-beta) * devRTT + beta * fabs(sampleRTT - roundTripTime);
            timeoutInterval = sampleRTT + 4 * devRTT;
        }

        //Go to next packet in the the linked list
        current_packet = current_packet->next;

        //Free the current compressed string
        free(final_string);
    }
    
    //Free the address info memory
    freeaddrinfo(res);

    //Close the socket file descriptor
    close(sockfd);
    
    return 0;
}
