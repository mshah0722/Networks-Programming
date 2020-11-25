#include "functions.h"

int main (int argc, char *argv[]){
	
    //see which and if user is logged in
    int user_logged_in = -1; 
	
	//see if joined session
    int session_joined = 0;  
    int sockfd;//socket file desriptor
    //printf(" ---> Welcome. ");

    fd_set read_fds;

BEGIN:

    while(1) {
		//Clear stream
        fflush(stdout);

		//clear entries
        FD_ZERO(&read_fds);
        FD_SET(STDIN_FILENO, &read_fds);

        int max_fd = STDIN_FILENO;
		
        if (user_logged_in >= 0) {
            if(sockfd > STDIN_FILENO) max_fd = sockfd;
            FD_SET(sockfd, &read_fds);
        }

        if (!session_joined) {
            if (user_logged_in >= 0) {
                // printf("Please select from one of the following commands:\n\t");
                // printf("/logout\n\t");
                // printf("/joinsession sessionID\n\t");
                // printf("/leavesession\n\t");
                // printf("/createsession sessionID\n\t");
                // printf("/list\n");
            }

            else{
                // printf("Please choose to login or quit:");
                // printf("\n\t/login id password serverIPAddress serverPort");
                // printf("\n\t/quit\n");
            }
        }

        if (select(max_fd + 1, &read_fds, NULL, NULL, NULL) < 0) {
            // printf("No actions in 10 seconds.");

            // if (user_logged_in < 0) {
                // goto BEGIN;
            // }

            // else{
                // printf(" The user has been logged out.");
                // goto log_out;
            // }
            
        }

        Message client_message;//message to send to server
        Message server_message;//message recieved from server

		//get inputs and read 
        if (FD_ISSET(STDIN_FILENO, &read_fds)) {
            char input[1000];
            char message_line[500];
            memset(input, 0, 1000);
            memset(message_line, 0, 500);

            fgets(input, 1000, stdin);
            strcpy(message_line, input);

            int length = strlen(message_line);
            message_line[length-1] = '\0';
			
            char *inputPtr;
            inputPtr = strtok(input, " \n");//read the command

            if (strcmp(inputPtr, "/login") == 0) {//login command
                
                int port = 0;

				//reserve space
                //int digit = 0;
                char *id_str = malloc(10);
                memset(id_str, 0, 10);
                char *address_str = malloc(20);
                memset(address_str, 0, 20);
                char *port_str = malloc(5);
                memset(port_str, 0, 5);
                int i = 0;

                client_message.type = LOGIN;    //set message type
                client_message.size = 10;		

                char login_password[11]; 	//stores password
                int index = 1;

                while (inputPtr != NULL) { //loop till the whole input isnt read
                    if (index == 1) { // get client id
                        inputPtr = strtok(NULL, " \n");
                        strcpy(id_str, inputPtr);
                        index++;
                    }

                    else if (index == 2) { // get password
                        inputPtr = strtok(NULL, " \n");
                        strcpy(login_password, inputPtr);
                        index++;
                    }

                    else if (index == 3) { // get server IP
                        inputPtr = strtok(NULL, " \n");
                        strcpy(address_str, inputPtr);
                        index++;
                    }

                    else if (index == 4) { // get server port
                        inputPtr = strtok(NULL, " \n");
                        strcpy(port_str, inputPtr);
                        break;
                    }
                }
				
				
				//port string to int
                port = atoi(port_str);


				//Creates a ipv4 dgram udp socket
                sockfd = socket(AF_INET, SOCK_STREAM, 0);
                if (sockfd < 0) fprintf(stderr, "socket error\n");

                struct sockaddr_in *server_addr;
                server_addr = (struct sockaddr_in *)malloc(sizeof(struct sockaddr_in));
                memset((char *)server_addr, 0, sizeof(struct sockaddr_in));

                server_addr->sin_addr.s_addr = inet_addr(address_str);
                server_addr->sin_port = htons(port);//set port to network byte order
                server_addr->sin_family = AF_INET;//sets to IPV4

                int conn_fd = connect(sockfd, (const struct sockaddr *)server_addr, sizeof(struct sockaddr_in));
                if (conn_fd != 0) {
                    printf("Error: Connection Failed\n");
                    exit(0);
                }

                // setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO,&timeout,sizeof(timeout));

                strcpy(client_message.source, id_str);
                strcpy(client_message.data, login_password);

                char type_string[5];
                char size_string[5];
                sprintf(type_string, "%d", client_message.type);
                sprintf(size_string, "%d", client_message.size);

                char *user_message = struct_to_string(type_string, size_string, client_message.source, client_message.data);
                
                write(sockfd, user_message, 1000);
                free(id_str);
                free(address_str);
                free(port_str);
                free(user_message);
            }

            else if (strcmp(inputPtr, "/createsession") == 0){
                if (user_logged_in < 0) {
                    printf(" ---> Sorry, the user has not logged in yet.\n");
                    goto BEGIN;
                }
                
                client_message.type = NEW_SESS;
                sprintf(client_message.source, "%d", user_logged_in);
                
                inputPtr = strtok(NULL, " \n");
                strcpy(client_message.data, inputPtr);
                client_message.size = strlen(client_message.data);

                char type_string[5];
                char size_string[5];
                sprintf(type_string, "%d", client_message.type);
                sprintf(size_string, "%d", client_message.size);

                char *user_message = struct_to_string(type_string, size_string, client_message.source, client_message.data);
                write(sockfd, user_message, 1000);
                free(user_message);
            }
            
            else if (strcmp(inputPtr, "/joinsession") == 0){
                if (user_logged_in < 0) {
                    printf(" ---> Sorry, the user has not logged in yet.\n");
                    goto BEGIN;
                }

                client_message.type = JOIN;
                sprintf(client_message.source, "%d", user_logged_in);

                inputPtr = strtok(NULL, " \n");
                strcpy(client_message.data, inputPtr);
                client_message.size = strlen(client_message.data);

                char type_string[5];
                char size_string[5];
                sprintf(type_string, "%d", client_message.type);
                sprintf(size_string, "%d", client_message.size);

                char *user_message = struct_to_string(type_string, size_string, client_message.source, client_message.data);
                write(sockfd, user_message, 1000);
                free(user_message);
            }
            
            else if (strcmp(inputPtr, "/leavesession") == 0){
                if (user_logged_in < 0) {
                    printf(" ---> Sorry, the user has not logged in yet.\n");
                    goto BEGIN;
                }

                client_message.type = LEAVE_SESS;
                client_message.size = 0;
                sprintf(client_message.source, "%d", user_logged_in);
                strcpy(client_message.data, " ");

                char type_string[5];
                char size_string[5];
                sprintf(type_string, "%d", client_message.type);
                sprintf(size_string, "%d", client_message.size);

                char *user_message = struct_to_string(type_string, size_string, client_message.source, client_message.data);
                
                write(sockfd, user_message, 1000);
                free(user_message);

                session_joined = 0;
                goto BEGIN;
            }
            
            else if (strcmp(inputPtr, "/list") == 0){
                if (user_logged_in < 0) {
                    printf(" ---> Sorry, the user has not logged in yet.\n");
                    goto BEGIN;
                }

                client_message.type = QUERY;
                client_message.size = 0;
                sprintf(client_message.source, "%d", user_logged_in);
                strcpy(client_message.data, " ");

                char type_string[5];
                char size_string[5];
                sprintf(type_string, "%d", client_message.type);
                sprintf(size_string, "%d", client_message.size);

                char *user_message = struct_to_string(type_string, size_string, client_message.source, client_message.data);
                
                write(sockfd, user_message, 1000);
                free(user_message);
            }
            

            else if (strcmp(inputPtr, "/logout") == 0){
LOG_OUT:
                if (user_logged_in < 0) {
                    printf(" ---> Sorry, the user has not logged in yet.\n");
                    goto BEGIN;
                }

                client_message.type = EXIT;
                client_message.size = 0;
                sprintf(client_message.source, "%d", user_logged_in);
                strcpy(client_message.data, " ");

                char type_string[5];
                char size_string[5];
                sprintf(type_string, "%d", client_message.type);
                sprintf(size_string, "%d", client_message.size);

                char *user_message = struct_to_string(type_string, size_string, client_message.source, client_message.data);
                
                write(sockfd, user_message, 1000);
                free(user_message);

                close(sockfd);
                session_joined = 0;
                user_logged_in = -1;
                max_fd = STDIN_FILENO;
                FD_CLR(sockfd, &read_fds);
                goto BEGIN;
            }
            

            else if (strcmp(inputPtr, "/quit") == 0) {

                if (user_logged_in >= 0) {
                    printf(" ---> Sorry, please logout first before quitting.\n");
                    goto BEGIN;
                }

                exit(0);
            }

            else {
                if (session_joined) {
                    client_message.type = MESSAGE;
                    sprintf(client_message.source, "%d", user_logged_in);

                    strcpy(client_message.data, message_line);
                    client_message.size = strlen(client_message.data);

                    char type_string[5];
                    char size_string[5];
                    sprintf(type_string, "%d", client_message.type);
                    sprintf(size_string, "%d", client_message.size);

                    char *user_message = struct_to_string(type_string, size_string, client_message.source, client_message.data);
                    write(sockfd, user_message, 1000);
                    free(user_message);
                }
                    
                else {
                    printf(" ---> Invalid command. ");
                    goto BEGIN;
                }
            }
        } 

        char buffer[1000];
        memset(buffer, 0, 1000);
        read(sockfd, buffer, 1000);

        char *inputPtr = strtok(buffer, ":");
        server_message.type = atoi(inputPtr); // type
        int index = 1;

        while (inputPtr != NULL) {
            if (index == 1) { // size
                inputPtr = strtok(NULL, ":");
                server_message.size = atoi(inputPtr);
                index++;
            }

            else if (index == 2) { // source
                inputPtr = strtok(NULL, ":");
                strcpy(server_message.source, inputPtr);
                index++;
            }

            else if (index == 3) { // data
                inputPtr = strtok(NULL, ":");
                strcpy(server_message.data, inputPtr);
                break;
            }
        }

        if (server_message.type == LO_ACK){
            printf(" ---> Login successful.\n");
            user_logged_in = (int) client_message.source[5] - 48;
        }

        else if (server_message.type == LO_NAK){
            printf(" ---> Login failed: %s", server_message.data);
            close(sockfd);
        }

        else if (server_message.type == NS_ACK){
            printf(" ---> Session %s created.\n", client_message.data);
        }
        
        else if (server_message.type == JN_ACK){
            printf(" ---> Session %s joined.\n", client_message.data);
            session_joined = 1;
        }

        else if (server_message.type == JN_NAK){
            printf(" ---> Failed to join session %s. %s\n", client_message.data, server_message.data);
        }
        
        else if (server_message.type == QU_ACK){
            printf(" ---> List of users and sessions:\n\t%s\n\n", server_message.data);
        }

        else if (server_message.type == MESSAGE){
            if (atoi(server_message.source) != user_logged_in) printf("guest%s: \"%s\"\n", server_message.source, server_message.data);
        }
    }
    return 0;
}