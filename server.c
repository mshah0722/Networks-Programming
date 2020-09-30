#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <netdb.h>

#define MAXBUFLEN 100

int main(int argc, char const *argv[]){
    
    int port;
	struct sockaddr_in server_addr;
    char buf[MAXBUFLEN] = {0};
	int sockfd;//socket file descriptor
    struct sockaddr_in clientAddress;
	
	char *msg_yes[]="yes";
	char *msg_no[]="no";
	
	if(argc == 2) //check for correct arguments
		port = atoi(argv[1]);
	else{//print error message and return
		fprintf(stderr, "incorrect argument amount. usage: server <udp listen port>\n");
		return 1;
	}
	 
    
    //Creates a ipv4 dgram udp socket
    if ((sockfd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) == -1) {
        fprintf(stderr, "socket error\n");
        return 1;
    } 
    
    
    server_addr.sin_family = AF_INET;//sets to IPV4
    
    server_addr.sin_port = htons(port);//set port to network byte order
    
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY);// references  IP address (in Network Byte Order)
    
    memset(server_addr.sin_zero, 0, sizeof(server_addr.sin_zero));//makes sure the struct is empty
    
    
    if (bind(sockfd, (struct sockaddr *) &server_addr, sizeof(server_addr)) == -1 ) {//Assigns address to the socket
        fprintf(stderr, "bind error\n");
        return 1;
    }
    
    
    unsigned msg_len = sizeof(struct sockaddr_in);//will contain the length of the address received
    
    if(recvfrom(sockfd, buf, MAXBUFLEN, 0, (struct sockaddr*) &clientAddress, &msg_len) == -1) {//receive message from client
        fprintf(stderr, "recv error\n");
        return 1;
    }
    
    if (strcmp(buf, "ftp") == 0) {//compare the messages received
        if (sendto(sockfd, msg_yes, strlen("msg_yes"), 0, (struct sockaddr*) &clientAddress, msg_len) == -1) {//check if message sent
                fprintf(stderr, "Message not sent\n");
                return 1;
        }
        else if (sendto(sockfd, msg_no, strlen(msg_no), 0, (struct sockaddr*) &clientAddress, msg_len) == -1) {//check if message sent
            fprintf(stderr, "Message not sent\n");
            return 1;
        }
    }

    close(sockfd);//close connection
    return 0;
}
