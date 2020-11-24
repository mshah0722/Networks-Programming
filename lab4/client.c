#include "functions.h"

int main (int argc, char *argv[]){
    
    int logged_in = -1;
    int session_joined = 0;
    int socket_fd;
    printf(" ---> Welcome. ");

    fd_set read_fds;

while_begin:

    while(1) {

        fflush(stdout);

        FD_ZERO(&read_fds);
        FD_SET(STDIN_FILENO, &read_fds);

        int max_fd = STDIN_FILENO;

        if (logged_in >= 0) {
            if(socket_fd > STDIN_FILENO) max_fd = socket_fd;
            FD_SET(socket_fd, &read_fds);
        }

        if (!session_joined) {
            if (logged_in >= 0) {
                printf("Please select from one of the following commands:\n\t");
                printf("/logout\n\t");
                printf("/joinsession sessionID\n\t");
                printf("/leavesession\n\t");
                printf("/createsession sessionID\n\t");
                printf("/list\n");
            }

            else{
                printf("Please choose to login or quit:");
                printf("\n\t/login id password serverIPAddress serverPort");
                printf("\n\t/quit\n");
            }
        }

        if (select(max_fd + 1, &read_fds, NULL, NULL, NULL) < 0) {
            printf("No actions in 10 seconds.");

            if (logged_in < 0) {
                goto while_begin;
            }

            else{
                printf(" The user has been logged out.");
                goto log_out;
            }
            
        }

        Message client_message;
        Message server_message;

        if (FD_ISSET(STDIN_FILENO, &read_fds)) {
            char input[1000];
            char message_line[500];
            memset(input, 0, 1000);
            memset(message_line, 0, 500);

            fgets(input, 1000, stdin);
            strcpy(message_line, input);

            int length = strlen(message_line);
            message_line[length-1] = '\0';

            char *token;
            token = strtok(input, " \n");

            if (strcmp(token, "/login") == 0) {
                
                int port = 0;

                int digit = 0;
                char *id_string = malloc(10);
                memset(id_string, 0, 10);
                char *address_string = malloc(20);
                memset(address_string, 0, 20);
                char *port_string = malloc(5);
                memset(port_string, 0, 5);
                int i = 0;

                client_message.type = LOGIN;    
                client_message.size = 10;

                char login_info[11];
                int index = 1;

                while (token != NULL) {
                    if (index == 1) { // client id
                        token = strtok(NULL, " \n");
                        strcpy(id_string, token);
                        index++;
                    }

                    else if (index == 2) { // password
                        token = strtok(NULL, " \n");
                        strcpy(login_info, token);
                        index++;
                    }

                    else if (index == 3) { // server-IP
                        token = strtok(NULL, " \n");
                        strcpy(address_string, token);
                        index++;
                    }

                    else if (index == 4) { // server-port
                        token = strtok(NULL, " \n");
                        strcpy(port_string, token);
                        break;
                    }
                }

                port = atoi(port_string);

                socket_fd = socket(AF_INET, SOCK_STREAM, 0);
                if (socket_fd < 0) fprintf(stderr, "socket error\n");

                struct sockaddr_in *server_addr;
                server_addr = (struct sockaddr_in *)malloc(sizeof(struct sockaddr_in));
                memset((char *)server_addr, 0, sizeof(struct sockaddr_in));

                server_addr->sin_addr.s_addr = inet_addr(address_string);
                server_addr->sin_port = htons(port);
                server_addr->sin_family = AF_INET;

                int conn_fd = connect(socket_fd, (const struct sockaddr *)server_addr, sizeof(struct sockaddr_in));
                if (conn_fd != 0) {
                    printf("Error: Connection Failed\n");
                    exit(0);
                }

                // setsockopt(socket_fd, SOL_SOCKET, SO_RCVTIMEO,&timeout,sizeof(timeout));

                strcpy(client_message.source, id_string);
                strcpy(client_message.data, login_info);

                char type_string[5];
                char size_string[5];
                sprintf(type_string, "%d", client_message.type);
                sprintf(size_string, "%d", client_message.size);

                char *user_message = Concat(type_string, size_string, client_message.source, client_message.data);
                
                write(socket_fd, user_message, 1000);
                free(id_string);
                free(address_string);
                free(port_string);
                free(user_message);
            }

            else if (strcmp(token, "/createsession") == 0){
                if (logged_in < 0) {
                    printf(" ---> Sorry, the user has not logged in yet.\n");
                    goto while_begin;
                }
                
                client_message.type = NEW_SESS;
                sprintf(client_message.source, "%d", logged_in);
                
                token = strtok(NULL, " \n");
                strcpy(client_message.data, token);
                client_message.size = strlen(client_message.data);

                char type_string[5];
                char size_string[5];
                sprintf(type_string, "%d", client_message.type);
                sprintf(size_string, "%d", client_message.size);

                char *user_message = Concat(type_string, size_string, client_message.source, client_message.data);
                write(socket_fd, user_message, 1000);
                free(user_message);
            }
            
            else if (strcmp(token, "/joinsession") == 0){
                if (logged_in < 0) {
                    printf(" ---> Sorry, the user has not logged in yet.\n");
                    goto while_begin;
                }

                client_message.type = JOIN;
                sprintf(client_message.source, "%d", logged_in);

                token = strtok(NULL, " \n");
                strcpy(client_message.data, token);
                client_message.size = strlen(client_message.data);

                char type_string[5];
                char size_string[5];
                sprintf(type_string, "%d", client_message.type);
                sprintf(size_string, "%d", client_message.size);

                char *user_message = Concat(type_string, size_string, client_message.source, client_message.data);
                write(socket_fd, user_message, 1000);
                free(user_message);
            }
            
            else if (strcmp(token, "/leavesession") == 0){
                if (logged_in < 0) {
                    printf(" ---> Sorry, the user has not logged in yet.\n");
                    goto while_begin;
                }

                client_message.type = LEAVE_SESS;
                client_message.size = 0;
                sprintf(client_message.source, "%d", logged_in);
                strcpy(client_message.data, " ");

                char type_string[5];
                char size_string[5];
                sprintf(type_string, "%d", client_message.type);
                sprintf(size_string, "%d", client_message.size);

                char *user_message = Concat(type_string, size_string, client_message.source, client_message.data);
                
                write(socket_fd, user_message, 1000);
                free(user_message);

                session_joined = 0;
                goto while_begin;
            }
            
            else if (strcmp(token, "/list") == 0){
                if (logged_in < 0) {
                    printf(" ---> Sorry, the user has not logged in yet.\n");
                    goto while_begin;
                }

                client_message.type = QUERY;
                client_message.size = 0;
                sprintf(client_message.source, "%d", logged_in);
                strcpy(client_message.data, " ");

                char type_string[5];
                char size_string[5];
                sprintf(type_string, "%d", client_message.type);
                sprintf(size_string, "%d", client_message.size);

                char *user_message = Concat(type_string, size_string, client_message.source, client_message.data);
                
                write(socket_fd, user_message, 1000);
                free(user_message);
            }
            

            else if (strcmp(token, "/logout") == 0){
log_out:
                if (logged_in < 0) {
                    printf(" ---> Sorry, the user has not logged in yet.\n");
                    goto while_begin;
                }

                client_message.type = EXIT;
                client_message.size = 0;
                sprintf(client_message.source, "%d", logged_in);
                strcpy(client_message.data, " ");

                char type_string[5];
                char size_string[5];
                sprintf(type_string, "%d", client_message.type);
                sprintf(size_string, "%d", client_message.size);

                char *user_message = Concat(type_string, size_string, client_message.source, client_message.data);
                
                write(socket_fd, user_message, 1000);
                free(user_message);

                close(socket_fd);
                session_joined = 0;
                logged_in = -1;
                max_fd = STDIN_FILENO;
                FD_CLR(socket_fd, &read_fds);
                goto while_begin;
            }
            

            else if (strcmp(token, "/quit") == 0) {

                if (logged_in >= 0) {
                    printf(" ---> Sorry, please logout first before quitting.\n");
                    goto while_begin;
                }

                exit(0);
            }

            else {
                if (session_joined) {
                    client_message.type = MESSAGE;
                    sprintf(client_message.source, "%d", logged_in);

                    strcpy(client_message.data, message_line);
                    client_message.size = strlen(client_message.data);

                    char type_string[5];
                    char size_string[5];
                    sprintf(type_string, "%d", client_message.type);
                    sprintf(size_string, "%d", client_message.size);

                    char *user_message = Concat(type_string, size_string, client_message.source, client_message.data);
                    write(socket_fd, user_message, 1000);
                    free(user_message);
                }
                    
                else {
                    printf(" ---> Invalid command. ");
                    goto while_begin;
                }
            }
        } 

        char buffer[1000];
        memset(buffer, 0, 1000);
        read(socket_fd, buffer, 1000);

        char *token = strtok(buffer, ":");
        server_message.type = atoi(token); // type
        int index = 1;

        while (token != NULL) {
            if (index == 1) { // size
                token = strtok(NULL, ":");
                server_message.size = atoi(token);
                index++;
            }

            else if (index == 2) { // source
                token = strtok(NULL, ":");
                strcpy(server_message.source, token);
                index++;
            }

            else if (index == 3) { // data
                token = strtok(NULL, ":");
                strcpy(server_message.data, token);
                break;
            }
        }

        if (server_message.type == LO_ACK){
            printf(" ---> Login successful.\n");
            logged_in = (int) client_message.source[5] - 48;
        }

        else if (server_message.type == LO_NAK){
            printf(" ---> Login failed: %s", server_message.data);
            close(socket_fd);
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
            if (atoi(server_message.source) != logged_in) printf("guest%s: \"%s\"\n", server_message.source, server_message.data);
        }
    }
    return 0;
}