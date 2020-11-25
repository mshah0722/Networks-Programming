#include "functions.h"

int main (int argc, char *argv[]){
	
    //see which and if user is logged in
    int user_logged_in = -1; 
	
	//see if joined session
    bool session_joined = false;  
    int sockfd;//socket file desriptor
    //printf(" ---> Welcome. ");

    fd_set read_fds;

BEGIN:

    while(true) {
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

        if (session_joined == false) {
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

        Message clientMsg;//message to send to server
        Message serverMsg;//message recieved from server

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

                clientMsg.type = LOGIN;    //set message type
                clientMsg.size = 10;		

                char login_password[11]; 	//stores password
                int index = 1;

                while (inputPtr != NULL) { //loop till the whole input is read
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
				
				//connect
                int conn_fd = connect(sockfd, (const struct sockaddr *)server_addr, sizeof(struct sockaddr_in));
                if (conn_fd != 0) {
                    printf("Connection error\n");
                    exit(0);
                }

				//store values in message struct 
                strcpy(clientMsg.source, id_str);
                strcpy(clientMsg.data, login_password);
				
				//store values in message struct in appropriate data type
                char typeStr[5];
                char sizeStr[5];
                sprintf(typeStr, "%d", clientMsg.type);
                sprintf(sizeStr, "%d", clientMsg.size);

				//compress string to send to client
                char *user_message = struct_to_string(typeStr, sizeStr, clientMsg.source, clientMsg.data);
                
				//send to server 
                write(sockfd, user_message, 1000);
				
				//free memory
                free(id_str);
                free(address_str);
                free(port_str);
                free(user_message);
            }

            else if (strcmp(inputPtr, "/createsession") == 0){ //createsession command
                if (user_logged_in < 0) {
                    printf("User not logged\n");
                    goto BEGIN;
                }
                
                clientMsg.type = NEW_SESS;//set msg tpye
                sprintf(clientMsg.source, "%d", user_logged_in);//set user id
                
                inputPtr = strtok(NULL, " \n");//read input
                strcpy(clientMsg.data, inputPtr);//store session name
                clientMsg.size = strlen(clientMsg.data);
				
				goto SEND_MSG;
				
				//store values in message struct in appropriate data type
                char typeStr[5];
                char sizeStr[5];
                sprintf(typeStr, "%d", clientMsg.type);
                sprintf(sizeStr, "%d", clientMsg.size);

				//compress string to send to client
                char *user_message = struct_to_string(typeStr, sizeStr, clientMsg.source, clientMsg.data);
				
				//send to server
                write(sockfd, user_message, 1000);
				
				//free memory
                free(user_message);
            }
            
            else if (strcmp(inputPtr, "/joinsession") == 0){// joinsession command
                if (user_logged_in < 0) {
                    printf("User not logged in\n");
                    goto BEGIN;
                }

                clientMsg.type = JOIN;//set msg type
                sprintf(clientMsg.source, "%d", user_logged_in);//set user id

                inputPtr = strtok(NULL, " \n");// read input
                strcpy(clientMsg.data, inputPtr);// store session name
                clientMsg.size = strlen(clientMsg.data);
				
				goto SEND_MSG;
				
				//store values in message struct in appropriate data type
                char typeStr[5];
                char sizeStr[5];
                sprintf(typeStr, "%d", clientMsg.type);
                sprintf(sizeStr, "%d", clientMsg.size);

				//compress string to send to client
                char *user_message = struct_to_string(typeStr, sizeStr, clientMsg.source, clientMsg.data);
				
				//send to server
                write(sockfd, user_message, 1000);
				
				//free memory
                free(user_message);
            }
            
            else if (strcmp(inputPtr, "/leavesession") == 0){//leavession command
                if (user_logged_in < 0) {
                    printf("User not logged in\n");
                    goto BEGIN;
                }

                clientMsg.type = LEAVE_SESS;//set msg type
                clientMsg.size = 0;
                sprintf(clientMsg.source, "%d", user_logged_in);//set user id
                strcpy(clientMsg.data, " ");
				
				//set to false
                session_joined = false;
				
				goto SEND_MSG;
				
				//store values in message struct in appropriate data type
                char typeStr[5];
                char sizeStr[5];
                sprintf(typeStr, "%d", clientMsg.type);
                sprintf(sizeStr, "%d", clientMsg.size);

				//compress string to send to client
                char *user_message = struct_to_string(typeStr, sizeStr, clientMsg.source, clientMsg.data);
                
				//send to server
                write(sockfd, user_message, 1000);	
				
				//free memory
                free(user_message);
				
                goto BEGIN;
            }
            
            else if (strcmp(inputPtr, "/list") == 0){//list command
                if (user_logged_in < 0) {
                    printf("User not logged in\n");
                    goto BEGIN;
                }

                clientMsg.type = QUERY;//set msg type
                clientMsg.size = 0;
                sprintf(clientMsg.source, "%d", user_logged_in);//set user id
                strcpy(clientMsg.data, " ");

				goto SEND_MSG;
				
				//store values in message struct in appropriate data type
                char typeStr[5];
                char sizeStr[5];
                sprintf(typeStr, "%d", clientMsg.type);
                sprintf(sizeStr, "%d", clientMsg.size);

				//compress string to send to client
                char *user_message = struct_to_string(typeStr, sizeStr, clientMsg.source, clientMsg.data);
				
				//send to server
                write(sockfd, user_message, 1000);
				
				//free memory
                free(user_message);
            }

	
            else if (strcmp(inputPtr, "/logout") == 0){//logout command
LOG_OUT:
                if (user_logged_in < 0) {
                    printf("User not logged in\n");
                    goto BEGIN;
                }

                clientMsg.type = EXIT;//set msg type
                clientMsg.size = 0;
                sprintf(clientMsg.source, "%d", user_logged_in);//set user id
                strcpy(clientMsg.data, " ");

				//store values in message struct in appropriate data type
                char typeStr[5];
                char sizeStr[5];
                sprintf(typeStr, "%d", clientMsg.type);
                sprintf(sizeStr, "%d", clientMsg.size);

				//compress string to send to client
                char *user_message = struct_to_string(typeStr, sizeStr, clientMsg.source, clientMsg.data);
                
				//send to server
                write(sockfd, user_message, 1000);
				
				//free memory
                free(user_message);

				//close socket
                close(sockfd);
				
				//reset values
                session_joined = false;
                user_logged_in = -1;
                max_fd = STDIN_FILENO;
                FD_CLR(sockfd, &read_fds);
                goto BEGIN;
            }
            

            else if (strcmp(inputPtr, "/quit") == 0) {//quit command

                if (user_logged_in >= 0) {
                    printf("Logout before quit\n");
                    goto BEGIN;
                }
				//terminate
                exit(0);
            }

            else {
                if (session_joined) {//<text>
                    clientMsg.type = MESSAGE;//set msg type
                    sprintf(clientMsg.source, "%d", user_logged_in);//set user id

                    strcpy(clientMsg.data, message_line);
                    clientMsg.size = strlen(clientMsg.data);//data to be sent
					
					goto SEND_MSG;
					
					//store values in message struct in appropriate data type
                    char typeStr[5];
                    char sizeStr[5];
                    sprintf(typeStr, "%d", clientMsg.type);
                    sprintf(sizeStr, "%d", clientMsg.size);

					//compress string to send to client
                    char *user_message = struct_to_string(typeStr, sizeStr, clientMsg.source, clientMsg.data);
					
					//send to server
                    write(sockfd, user_message, 1000);
					
					//free memory
                    free(user_message);
                }
                    
                else {
                    printf("Invalid command. ");
                    goto BEGIN;
                }
            }
SEND_MSG:	
				//store values in message struct in appropriate data type
                char typeStr[5];
                char sizeStr[5];
                sprintf(typeStr, "%d", clientMsg.type);
                sprintf(sizeStr, "%d", clientMsg.size);

				//compress string to send to client
                char *user_message = struct_to_string(typeStr, sizeStr, clientMsg.source, clientMsg.data);
				
				//send to server
                write(sockfd, user_message, 1000);
				
				//free memory
                free(user_message);
				
			if(clientMsg.type == LEAVE_SESS )goto BEGIN;
				
			
        } 

		//recieve message and break it down and assign it to variables
        char buffer[1000];
        memset(buffer, 0, 1000);
        read(sockfd, buffer, 1000);//read from the server

        char *inputPtr = strtok(buffer, ":");
        serverMsg.type = atoi(inputPtr); // msg type
        int index = 1;

		//loop till the whole input is read
        while (inputPtr != NULL) {
            if (index == 1) { // size
                inputPtr = strtok(NULL, ":");
                serverMsg.size = atoi(inputPtr);
                index++;
            }

            else if (index == 2) { // source
                inputPtr = strtok(NULL, ":");
                strcpy(serverMsg.source, inputPtr);
                index++;
            }

            else if (index == 3) { // data
                inputPtr = strtok(NULL, ":");
                strcpy(serverMsg.data, inputPtr);
                break;
            }
        }

        if (serverMsg.type == LO_ACK){// login ack
            printf("Login successful\n");
            user_logged_in = (int) clientMsg.source[5] - 48;//set which user logged in
        }

        else if (serverMsg.type == LO_NAK){//login nack
            printf("Login failed: %s", serverMsg.data);
            close(sockfd);
        }

        else if (serverMsg.type == NS_ACK){//newsession ack
            printf("Session %s created\n", clientMsg.data);
        }
        
        else if (serverMsg.type == JN_ACK){//joinsession ack
            printf("Session %s joined\n", clientMsg.data);
            session_joined = true;//set value since joined session
        }

        else if (serverMsg.type == JN_NAK){//joinsesion nack
            printf("Failed join session %s. %s\n", clientMsg.data, serverMsg.data);
        }
        
        else if (serverMsg.type == QU_ACK){//query ack
            printf("List of users and sessions:\n\t%s\n\n", serverMsg.data);
        }

        else if (serverMsg.type == MESSAGE){//<text> recieved
            if (atoi(serverMsg.source) != user_logged_in) printf("guest%s: \"%s\"\n", serverMsg.source, serverMsg.data);
        }
    }
    return 0;
}