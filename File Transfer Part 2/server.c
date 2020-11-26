#include "functions.h"

#define MAXBUFLEN 1100

int main(int argc, char const *argv[]){
    
    int port;
	struct sockaddr_in server_addr;
    char buf[MAXBUFLEN] = {0};
	int sockfd;//socket file descriptor
    struct sockaddr_in client_addr;
	
	const char msg_yes[] = "yes";
	const char msg_no[] = "no";
	const char msg_ACK[] = "ACK";
	
	if(argc == 2) //check for correct arguments
		port = atoi(argv[1]);
	else{//print error message and return
		fprintf(stderr, "Incorrect argument amount. usage: server <udp listen port>\n");
		return 1;
	}
	 
    
    //Creates a ipv4 dgram udp socket
    if ((sockfd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) == -1) {
        fprintf(stderr, "Socket error\n");
        return 1;
    } 
    
    
    server_addr.sin_family = AF_INET;//sets to IPV4
    
    server_addr.sin_port = htons(port);//set port to network byte order
    
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY);// references IP address (in Network Byte Order)
    
    memset(server_addr.sin_zero, 0, sizeof(server_addr.sin_zero));//makes sure the struct is empty
    
    
    if (bind(sockfd, (struct sockaddr *) &server_addr, sizeof(server_addr)) == -1 ) {//Assigns address to the socket
        fprintf(stderr, "Bind error\n");
        return 1;
    }
    
    
    unsigned msg_len = sizeof(struct sockaddr_in);//will contain the length of the address received
    
    if(recvfrom(sockfd, buf, MAXBUFLEN, 0, (struct sockaddr*) &client_addr, &msg_len) == -1) {//receive message from client
        fprintf(stderr, "Recv error\n");
        return 1;
    }
    
    if (strcmp(buf, "ftp") == 0) {//compare the messages received
        if (sendto(sockfd, msg_yes, strlen("msg_yes"), 0, (struct sockaddr*) &client_addr, msg_len) == -1) {//check if message sent
                fprintf(stderr, "Message not sent\n");
                return 1;
        }
        else if (sendto(sockfd, msg_no, strlen(msg_no), 0, (struct sockaddr*) &client_addr, msg_len) == -1) {//check if message sent
            fprintf(stderr, "Message not sent\n");
            return 1;
        }
    }

    FILE* fp;
    int loop = 1;
    while(loop == 1) {
		
        //receive packets in string format
        int bytes_recv = recvfrom(sockfd, buf, MAXBUFLEN, 0, (struct sockaddr*) &client_addr, &msg_len);
        buf[bytes_recv] = '\0';//add null character
        
        //Send ACK
        if (sendto(sockfd, msg_ACK, strlen(msg_ACK), 0, (struct sockaddr*) &client_addr, msg_len) == -1) {
            fprintf(stderr, "Acknowledgement message not sent to client\n");
            exit(1);
        }

        //extracting string and store into struct format
        struct packet* current_packet = string_to_struct(buf);
        int total_frag = current_packet->total_frag;
        int frag_no = current_packet->frag_no;
        int size = current_packet->size;
        char* filename = current_packet->filename;
        char* filedata = current_packet->filedata;
		
        if (frag_no == total_frag) loop = 0;//check loop condition
		

        if(frag_no == 1) {
            fp = fopen(filename, "wb");//open/create file in write binary
        }
        fwrite(filedata, 1, size, fp);//write to file
        
        
        //Freeing the current packet pointer
        free(current_packet);
        
    }
    fclose(fp);//close file

    close(sockfd);//close connection
    return 0;
}
