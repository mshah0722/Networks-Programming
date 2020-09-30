#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <netdb.h>

#define MAXLEN 4096

int main(int argc, char const *argv[]){
    
    int port;
    
    // switch(argc) {
        // case 2:
            // port = atoi(argv[1]);
            // break;
        // default:
            // fprintf(stderr, "usage: server (udp listen port)\n");
            // return(0);
    // }
    
	if(argc == 2) //check for correct arguments
		port = atoi(argv[1]);
	else{//print error message and return
		printf(stderr, "incorrect arguments amoung. usage: server <udp listen port>\n");
		return 1;
	}
	
    int socketDescriptor;
    
    //Creates a ipv4 dgram udp socket
    if ((socketDescriptor = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) == -1) {
        fprintf(stderr, "socket error\n");
        return 1;
    }
    
    struct sockaddr_in serverAddress; 
    
    
    serverAddress.sin_family = AF_INET;//sets to IPV4
    
    serverAddress.sin_port = htons(port);//set port to network byte order
    
    serverAddress.sin_addr.s_addr = htonl(INADDR_ANY);// references  IP address (in Network Byte Order)
    //Sets it to the right memory space
    memset(serverAddress.sin_zero, 0, sizeof(serverAddress.sin_zero));
    
    //Assigns address to the socket
    if (bind(socketDescriptor, (struct sockaddr *) &serverAddress, sizeof(serverAddress)) == -1 ) {
        fprintf(stderr, "Can't bind name to socket\n");
        return 1;
    }
    
    
    char message_buffer[MAXLEN] = {0};
    struct sockaddr_in clientAddress;
    unsigned clientMessageLength = sizeof(struct sockaddr_in);
    
    if(recvfrom(socketDescriptor, message_buffer, MAXLEN, 0, (struct sockaddr*) &clientAddress, &clientMessageLength) < 0) {
        fprintf(stderr, "Can't receive data\n");
        return 1;
    }
    
    if (strcmp(message_buffer, "ftp") == 0) {
        if (sendto(socketDescriptor, "yes", strlen("yes"), 0, (struct sockaddr*) &clientAddress, clientMessageLength) == -1) {
                fprintf(stderr, "Message not sent to client\n");
                return(1);
        }
        else if (sendto(socketDescriptor, "no", strlen("no"), 0, (struct sockaddr*) &clientAddress, clientMessageLength) == -1) {
            fprintf(stderr, "Message not sent to client\n");
            return 1;
        }
    }

    close(socketDescriptor);
    return 0;
}
