#include "functions.h"

#define NUM_CLIENT 5

// list of client access
ID_Pass client_list[NUM_CLIENT];

void Print_Login_Status() {

    printf("User status:");

    for (int i = 0; i < NUM_CLIENT; i++) {
        printf(" %d->", i);
        if (client_list[i].logged_in) printf("on");
        else printf("off");
    }

    printf("\n");
}

void Print_User_Session (char **session_list) {

    printf("User-session status:");

    for (int i = 0; i < NUM_CLIENT; i++){
        if (client_list[i].logged_in != 0){
            int session_index = client_list[i].session_id;
            printf(" %d->", i);
            
            if (session_index == -1) {
                printf("N/A");
            }

            else {
                printf("%s", session_list[session_index]);
            }
        }
    }
    
    printf("\n");
}

void Print_Session_Status (char **session_list) {

    printf("Session status:");

    for (int i = 0; i < NUM_CLIENT; i++){
        if (session_list[i] != NULL){
            printf(" %d->%s", i, session_list[i]);
        }
    }

    printf("\n");
}

int main (int argc, char *argv[]){

    int port = 0;

    if (argc != 2){
        fprintf(stderr, "Please enter the server port number.\n");
        exit(1);
    }

    else {
        int digit = 0;
        char *port_string = argv[1];
        int i = 0;

        while (port_string[i] != '\0'){
            digit = (int) port_string[i] - 48;
            port *= 10;
            port += digit;
            i++;
        }
    }

    // setting the credentials for each client
    for (int i = 0; i < NUM_CLIENT; i++) {
        strcpy(client_list[i].id, "guestX");
        client_list[i].id[5] = i + '0';
        char password[4];
        sprintf(password, "%d%d%d%d", i, i, i, i);
        strcpy(client_list[i].password, password);
        client_list[i].logged_in = 0;
        client_list[i].accept_fd = 0;
        client_list[i].session_id = -1;
    }

    for (int i = 0; i < NUM_CLIENT; i++) {
        printf("User No.%d: %s, password: %s\n", i + 1, client_list[i].id, client_list[i].password);
    }

    Print_Login_Status();

    char *session_list[NUM_CLIENT];
    for (int i = 0; i < NUM_CLIENT; i++) {
        session_list[i] = NULL;
    }
    
    // check if a TCP socket can be created
    int socket_fd = socket(AF_INET, SOCK_STREAM, 0);

    if (socket_fd < 0){
        fprintf(stderr, "socket error\n");
    }

    struct sockaddr_in *server_addr, *client_addr;
    server_addr = (struct sockaddr_in *)malloc(sizeof(struct sockaddr_in));
    client_addr = (struct sockaddr_in *)malloc(sizeof(struct sockaddr_in));
    memset(server_addr, 0, sizeof(struct sockaddr_in));
    memset(client_addr, 0, sizeof(struct sockaddr_in));

    // create a TCP socket
    server_addr->sin_addr.s_addr = INADDR_ANY;
    server_addr->sin_port = htons(port);
    server_addr->sin_family = AF_INET;
    unsigned int server_size = sizeof(struct sockaddr);

    int bind_fd = bind(socket_fd, (const struct sockaddr*)server_addr, sizeof(struct sockaddr));

    // assigning a name(binding) to the socket 
    if (bind_fd == -1) {
        fprintf(stderr, "bind error\n");
        exit(0);
    }

    fd_set listen_set;

    int listen_fd = listen(socket_fd, 10);

    if (listen_fd != 0) {
        fprintf(stderr, "listen error");
        exit(0);
    }

    while (1) {

        FD_ZERO(&listen_set);

        FD_SET(socket_fd, &listen_set);

        int max_fd = socket_fd;
        for (int i = 0; i < NUM_CLIENT; i++){
            if (max_fd < client_list[i].accept_fd) max_fd = client_list[i].accept_fd;

            if (client_list[i].logged_in != 0)
                FD_SET(client_list[i].accept_fd, &listen_set);
        }

        select(max_fd + 1, &listen_set, NULL, NULL, NULL);

        int ready_fd = socket_fd;
        for (int i = 0; i < NUM_CLIENT; i++){
            if (client_list[i].logged_in == 1) {
                if (FD_ISSET(client_list[i].accept_fd, &listen_set)) {
                    ready_fd = client_list[i].accept_fd;
                    break;
                }
            }
        }

        char buffer[1000];
        
        memset(buffer, 0, sizeof(buffer));

        if (ready_fd == socket_fd) { // new login request

            ready_fd = accept(socket_fd, (struct sockaddr*)client_addr, &server_size);
            if (ready_fd < 0) {
                fprintf(stderr, "server accept error");
                exit(0);
            }

        }
            
        read(ready_fd, buffer, 1000);
        
        printf("Message received from client ---> %s\n", buffer);

        char *token;
        token = strtok(buffer, ":");
        Message client_message;
        Message server_message;
        client_message.type = atoi(token); // type
        int index = 1;

        while (token != NULL) {
            if (index == 1) { // size
                token = strtok(NULL, ":");
                client_message.size = atoi(token);
                index++;
            }

            else if (index == 2) { // source
                token = strtok(NULL, ":");
                strcpy(client_message.source, token);
                index++;
            }

            else if (index == 3) { // data
                token = strtok(NULL, ":");
                strcpy(client_message.data, token);
                break;
            }
        }

        memset(buffer, 0, sizeof(buffer));

        if (client_message.type == LOGIN) {

            int flag = 0;
            int index;

            for (index = 0; index < NUM_CLIENT; index++) {
                if ((strcmp(client_list[index].id, client_message.source) == 0) && (strcmp(client_list[index].password, client_message.data) == 0)) {
                    
                    if (client_list[index].logged_in) {
                        flag = -1;
                    }

                    else {
                        flag = 1;
                        client_list[index].logged_in = 1;
                        client_list[index].accept_fd = ready_fd;
                    }

                    break;
                }
            }

            if (flag == 1) {
                server_message.type = LO_ACK;
                server_message.size = 0;
                strcpy(server_message.source, client_message.source);
                strcpy(server_message.data, "Success");
                char type_string[5];
                char size_string[5];
                sprintf(type_string, "%d", server_message.type);
                sprintf(size_string, "%d", server_message.size);
                char *serv_message = Concat(type_string, size_string, server_message.source, server_message.data);
                printf("User %d logged in. ", index);
                Print_Login_Status();
                write(ready_fd, serv_message, 1000);
                free(serv_message);
            }

            else if (flag == 0){
                server_message.type = LO_NAK;
                server_message.size = 100;
                strcpy(server_message.source, client_message.source);
                strcpy(server_message.data, "User id/password is incorrect, please retry.\n");
                char type_string[5];
                char size_string[5];
                sprintf(type_string, "%d", server_message.type);
                sprintf(size_string, "%d", server_message.size);
                char *serv_message = Concat(type_string, size_string, server_message.source, server_message.data);
                write(ready_fd, serv_message, 1000);
                free(serv_message);
                close(ready_fd);
            }

            else{
                server_message.type = LO_NAK;
                server_message.size = 100;
                strcpy(server_message.source, client_message.source);
                strcpy(server_message.data, "This user has already logged, please try a different one.\n");
                char type_string[5];
                char size_string[5];
                sprintf(type_string, "%d", server_message.type);
                sprintf(size_string, "%d", server_message.size);
                char *serv_message = Concat(type_string, size_string, server_message.source, server_message.data);
                write(ready_fd, serv_message, 1000);
                free(serv_message);
                close(ready_fd);
            }
        }

        else if (client_message.type == NEW_SESS){
            int client_index = atoi(client_message.source);

            for (int i = 0; i < NUM_CLIENT; i++){
                if (session_list[i] == NULL) {
                    session_list[i] = (char *)malloc(100);
                    strcpy(session_list[i], client_message.data);
                    break;
                }
            }
            
            Print_Session_Status(session_list);

            server_message.type = NS_ACK;
            server_message.size = 0;
            strcpy(server_message.source, client_message.source);
            strcpy(server_message.data, " ");

            char type_string[5];
            char size_string[5];
            sprintf(type_string, "%d", server_message.type);
            sprintf(size_string, "%d", server_message.size);
            char *serv_message = Concat(type_string, size_string, server_message.source, server_message.data);
            write(ready_fd, serv_message, 1000);
            free(serv_message);
        }

        else if (client_message.type == JOIN){
            int client_index = atoi(client_message.source);
            int success = 0;

            for (int i = 0; i < NUM_CLIENT; i++){
                if (session_list[i] != NULL) {
                    if (strcmp(session_list[i], client_message.data) == 0){
                        client_list[client_index].session_id = i;
                        success = 1;
                        break;
                    }
                }
            }

            if (success) {
                printf("User %d joined %s. ", client_index, session_list[client_list[client_index].session_id]);
                Print_User_Session(session_list);

                server_message.type = JN_ACK;
                server_message.size = 0;
                strcpy(server_message.source, client_message.source);
                strcpy(server_message.data, " ");
            }

            else{
                printf("User %d failed to join %s. ", client_index, client_message.data);
                Print_User_Session(session_list);

                server_message.type = JN_NAK;
                server_message.size = 0;
                strcpy(server_message.source, client_message.source);
                strcpy(server_message.data, "Session does not exist.");
            }

            char type_string[5];
            char size_string[5];
            sprintf(type_string, "%d", server_message.type);
            sprintf(size_string, "%d", server_message.size);
            char *serv_message = Concat(type_string, size_string, server_message.source, server_message.data);
            write(ready_fd, serv_message, 1000);
            free(serv_message);
        }
        
        else if (client_message.type == LEAVE_SESS){
            int client_index = atoi(client_message.source);

            printf("User %d left session %s. ", client_index, session_list[client_list[client_index].session_id]);
            client_list[client_index].session_id = -1;
            Print_User_Session(session_list);

        }
        
        else if (client_message.type == QUERY){
            int client_index = atoi(client_message.source);

            server_message.type = QU_ACK;
            strcpy(server_message.source, client_message.source);
            
            memset(server_message.data, 0, MAX_DATA);

            for (int i = 0; i < NUM_CLIENT; i++){
                if (session_list[i] != NULL){
                    char temp[100];
                    sprintf(temp, "%s ", session_list[i]);
                    strcat(server_message.data, temp);
                }
            }

            strcat(server_message.data, "\n\t");

            for (int i = 0; i < NUM_CLIENT; i++){
                if (client_list[i].logged_in != 0){
                    int session_index = client_list[i].session_id;
                    char temp[100];
                    char sess_name[20];
                    sprintf(temp, "%d->", i);
                    
                    if (session_index == -1) {
                        strcpy(sess_name, "N/A ");
                    }

                    else {
                        sprintf(sess_name, "%s ", session_list[session_index]);
                    }

                    strcat(temp, sess_name);
                    strcat(server_message.data, temp);
                }
            }

            server_message.size = strlen(server_message.data);

            char type_string[5];
            char size_string[5];
            sprintf(type_string, "%d", server_message.type);
            sprintf(size_string, "%d", server_message.size);
            char *serv_message = Concat(type_string, size_string, server_message.source, server_message.data);
            write(ready_fd, serv_message, 1000);
            free(serv_message);
        }

        else if (client_message.type == MESSAGE){
            int client_index = atoi(client_message.source);
            int session_index = client_list[client_index].session_id;

            server_message.type = MESSAGE;
            strcpy(server_message.source, client_message.source);
            strcpy(server_message.data, client_message.data);

            server_message.size = strlen(server_message.data);

            char type_string[5];
            char size_string[5];
            sprintf(type_string, "%d", server_message.type);
            sprintf(size_string, "%d", server_message.size);

            char *serv_message = Concat(type_string, size_string, server_message.source, server_message.data);

            for (int i = 0; i < NUM_CLIENT; i++){
                if (client_list[i].session_id == session_index){
                    write(client_list[i].accept_fd, serv_message, 1000);
                }
            }
            
            free(serv_message);
        }     

        else if (client_message.type == EXIT){
            int client_index = atoi(client_message.source);

            client_list[client_index].logged_in = 0;
            printf("User %d quitted. ", client_index);
            Print_Login_Status();

            close(client_list[client_index].accept_fd);
            client_list[client_index].logged_in = 0;
            client_list[client_index].accept_fd = 0;
            client_list[client_index].session_id = -1;
        }
    }

    return 0;
}