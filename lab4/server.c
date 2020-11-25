#include "functions.h"

// List of client access
ID_Pass listOfClients[NUM_CLIENT];

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

    // Setting the credentials for each client
    for (int i = 0; i < NUM_CLIENT; i++) {
        strcpy(listOfClients[i].id, "User_X");
        listOfClients[i].id[5] = i + '0';
        char password[4];
        sprintf(password, "%d%d%d%d", i, i, i, i);
        strcpy(listOfClients[i].password, password);
        listOfClients[i].logged_in = 0;
        listOfClients[i].accept_fd = 0;
        listOfClients[i].session_id = -1;
    }

    for (int i = 0; i < NUM_CLIENT; i++) {
        printf("Client No.%d: %s, password: %s\n", i + 1, listOfClients[i].id, listOfClients[i].password);
    }

    displayLoginStatus();

    char *session_list[NUM_CLIENT];
    for (int i = 0; i < NUM_CLIENT; i++) {
        session_list[i] = NULL;
    }
    
    // check if a TCP socket can be created
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);

    if (sockfd < 0){
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

    int bindfd = bind(sockfd, (const struct sockaddr*)server_addr, sizeof(struct sockaddr));

    // assigning a name(binding) to the socket 
    if (bindfd == -1) {
        fprintf(stderr, "bind error\n");
        exit(0);
    }

    fd_set listen_set;

    int listenfd = listen(sockfd, 10);

    if (listenfd != 0) {
        fprintf(stderr, "listen error");
        exit(0);
    }

    while (1) {

        FD_ZERO(&listen_set);

        FD_SET(sockfd, &listen_set);

        int max_fd = sockfd;
        for (int i = 0; i < NUM_CLIENT; i++){
            if (max_fd < listOfClients[i].accept_fd) max_fd = listOfClients[i].accept_fd;

            if (listOfClients[i].logged_in != 0)
                FD_SET(listOfClients[i].accept_fd, &listen_set);
        }

        select(max_fd + 1, &listen_set, NULL, NULL, NULL);

        int ready_fd = sockfd;
        for (int i = 0; i < NUM_CLIENT; i++){
            if (listOfClients[i].logged_in == 1) {
                if (FD_ISSET(listOfClients[i].accept_fd, &listen_set)) {
                    ready_fd = listOfClients[i].accept_fd;
                    break;
                }
            }
        }

        char buffer[1000];
        
        memset(buffer, 0, sizeof(buffer));

        if (ready_fd == sockfd) { // new login request

            ready_fd = accept(sockfd, (struct sockaddr*)client_addr, &server_size);
            if (ready_fd < 0) {
                fprintf(stderr, "server accept error");
                exit(0);
            }

        }
            
        read(ready_fd, buffer, 1000);
        
        printf("Message received from client ---> %s\n", buffer);

        char *inputPtr;
        inputPtr = strtok(buffer, ":");
        Message client_message;
        Message server_message;
        client_message.type = atoi(inputPtr); // type
        int index = 1;

        while (inputPtr != NULL) {
            if (index == 1) { // size
                inputPtr = strtok(NULL, ":");
                client_message.size = atoi(inputPtr);
                index++;
            }

            else if (index == 2) { // source
                inputPtr = strtok(NULL, ":");
                strcpy(client_message.source, inputPtr);
                index++;
            }

            else if (index == 3) { // data
                inputPtr = strtok(NULL, ":");
                strcpy(client_message.data, inputPtr);
                break;
            }
        }

        memset(buffer, 0, sizeof(buffer));

        if (client_message.type == LOGIN) {

            int flag = 0;
            int index;

            for (index = 0; index < NUM_CLIENT; index++) {
                if ((strcmp(listOfClients[index].id, client_message.source) == 0) && (strcmp(listOfClients[index].password, client_message.data) == 0)) {
                    
                    if (listOfClients[index].logged_in) {
                        flag = -1;
                    }

                    else {
                        flag = 1;
                        listOfClients[index].logged_in = 1;
                        listOfClients[index].accept_fd = ready_fd;
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
                char *serv_message = struct_to_string(type_string, size_string, server_message.source, server_message.data);
                printf("User %d logged in. ", index);
                displayLoginStatus();
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
                char *serv_message = struct_to_string(type_string, size_string, server_message.source, server_message.data);
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
                char *serv_message = struct_to_string(type_string, size_string, server_message.source, server_message.data);
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
            
            displaySessionStatus(session_list);

            server_message.type = NS_ACK;
            server_message.size = 0;
            strcpy(server_message.source, client_message.source);
            strcpy(server_message.data, " ");

            char type_string[5];
            char size_string[5];
            sprintf(type_string, "%d", server_message.type);
            sprintf(size_string, "%d", server_message.size);
            char *serv_message = struct_to_string(type_string, size_string, server_message.source, server_message.data);
            write(ready_fd, serv_message, 1000);
            free(serv_message);
        }

        else if (client_message.type == JOIN){
            int client_index = atoi(client_message.source);
            int success = 0;

            for (int i = 0; i < NUM_CLIENT; i++){
                if (session_list[i] != NULL) {
                    if (strcmp(session_list[i], client_message.data) == 0){
                        listOfClients[client_index].session_id = i;
                        success = 1;
                        break;
                    }
                }
            }

            if (success) {
                printf("User %d joined %s. ", client_index, session_list[listOfClients[client_index].session_id]);
                displayUserSession(session_list);

                server_message.type = JN_ACK;
                server_message.size = 0;
                strcpy(server_message.source, client_message.source);
                strcpy(server_message.data, " ");
            }

            else{
                printf("User %d failed to join %s. ", client_index, client_message.data);
                displayUserSession(session_list);

                server_message.type = JN_NAK;
                server_message.size = 0;
                strcpy(server_message.source, client_message.source);
                strcpy(server_message.data, "Session does not exist.");
            }

            char type_string[5];
            char size_string[5];
            sprintf(type_string, "%d", server_message.type);
            sprintf(size_string, "%d", server_message.size);
            char *serv_message = struct_to_string(type_string, size_string, server_message.source, server_message.data);
            write(ready_fd, serv_message, 1000);
            free(serv_message);
        }
        
        else if (client_message.type == LEAVE_SESS){
            int client_index = atoi(client_message.source);

            printf("User %d left session %s. ", client_index, session_list[listOfClients[client_index].session_id]);
            listOfClients[client_index].session_id = -1;
            displayUserSession(session_list);

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
                if (listOfClients[i].logged_in != 0){
                    int session_index = listOfClients[i].session_id;
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
            char *serv_message = struct_to_string(type_string, size_string, server_message.source, server_message.data);
            write(ready_fd, serv_message, 1000);
            free(serv_message);
        }

        else if (client_message.type == MESSAGE){
            int client_index = atoi(client_message.source);
            int session_index = listOfClients[client_index].session_id;

            server_message.type = MESSAGE;
            strcpy(server_message.source, client_message.source);
            strcpy(server_message.data, client_message.data);

            server_message.size = strlen(server_message.data);

            char type_string[5];
            char size_string[5];
            sprintf(type_string, "%d", server_message.type);
            sprintf(size_string, "%d", server_message.size);

            char *serv_message = struct_to_string(type_string, size_string, server_message.source, server_message.data);

            for (int i = 0; i < NUM_CLIENT; i++){
                if (listOfClients[i].session_id == session_index){
                    write(listOfClients[i].accept_fd, serv_message, 1000);
                }
            }
            
            free(serv_message);
        }     

        else if (client_message.type == EXIT){
            int client_index = atoi(client_message.source);

            listOfClients[client_index].logged_in = 0;
            printf("User %d quitted. ", client_index);
            displayLoginStatus();

            close(listOfClients[client_index].accept_fd);
            listOfClients[client_index].logged_in = 0;
            listOfClients[client_index].accept_fd = 0;
            listOfClients[client_index].session_id = -1;
        }
    }

    return 0;
}