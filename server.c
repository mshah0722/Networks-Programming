#include <stdio.h> 
#include <stdlib.h> 
#include <unistd.h> 
#include <string.h> 
#include <sys/types.h> 
#include <sys/socket.h> 
#include <netinet/in.h>
#include <netdb.h>

int main(int argc, char **argv){
    
    struct sockaddr_in servaddr, cliaddr;
    struct addrinfo hints, *res, *p;
    struct addrinfo *servinfo; // will point to the results
    socklen_t clilen, addr_size;


    int status;
    int port = atoi(argv[1]); // get the port
    int sockfd = socket(AF_INET, SOCK_DGRAM, 0); // get the socket
    char *yes_msg = "yes";
    char *no_msg = "no";

    if (argc != 2) {
        fprintf(stderr,"usage: showip hostname\n");
        return 1;
    }

    // check for address
    if ((status = getaddrinfo(argv[1], NULL, &hints, &res)) != 0) {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(status));
        return 2;
    }
    
    // try opening the socket, print an error otherwise
    if (sockfd < 0) { 
        perror("failed to create a socket!!"); 
        exit(1); 
    }

    // set port numbers (stuff that is fixed)
    servaddr.sin_family    = AF_INET; // IPv4 
    servaddr.sin_addr.s_addr = INADDR_ANY; 
    servaddr.sin_port = htons(port); 
    memset(servaddr.sin_zero, '\0', sizeof servaddr.sin_zero);

    int binder = bind(sockfd, (struct sockaddr *)&servaddr, sizeof servaddr);
    // check for bind error
    if (binder == -1) {
        close(sockfd);
        perror("failed to bind!!");
        exit(1);
    }
    
    listen(sockfd, BACKLOG);
    addr_size = sizeof servaddr;
    int new_fd = accept(sockfd, (struct sockaddr *)&servaddr, &addr_size);
    if (new_fd == -1) {
        perror("failed to accept");
        exit(1);
    }

    int BUF_SIZE = 50; // len is the maximum length of the buffer which we don't know

    char buffer[BUF_SIZE]; // = 0; // buf is the buffer to read the information into. Gonna store stuff
    int received = recvfrom(sockfd, buffer, BUF_SIZE, 0, (struct sockaddr *) &cliaddr, &clilen); 
    if(received == -1){
        perror("failed to receive from client");
        exit(1);
    }

    if(strcmp(buffer, "ftp")){
        int sentYes = sendto(sockfd, yes_msg, strlen(yes_msg), 0, (struct sockaddr *) &cliaddr, sizeof(cliaddr));
        if(sentYes == -1){
            perror("failed to send yes");
            exit(1);
        }
            
    } else{
        int sentNo = sendto(sockfd, no_msg, strlen(no_msg), 0, (struct sockaddr *) &cliaddr, sizeof(cliaddr));
        if(sentNo == -1){
            perror("failed to send no");
            exit(1);
        }
    }
    
    // close up
    close(sockfd); 

    return 0;
}