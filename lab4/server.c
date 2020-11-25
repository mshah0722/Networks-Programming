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
        Message clientMsg;
        Message serverMsg;
        clientMsg.type = atoi(inputPtr); // type
        int index = 1;

        while (inputPtr != NULL) {
            if (index == 1) { // size
                inputPtr = strtok(NULL, ":");
                clientMsg.size = atoi(inputPtr);
                index++;
            }

            else if (index == 2) { // source
                inputPtr = strtok(NULL, ":");
                strcpy(clientMsg.source, inputPtr);
                index++;
            }

            else if (index == 3) { // data
                inputPtr = strtok(NULL, ":");
                strcpy(clientMsg.data, inputPtr);
                break;
            }
        }

        memset(buffer, 0, sizeof(buffer));

        if (clientMsg.type == LOGIN) {

            int flag = 0;
            int index;

            for (index = 0; index < NUM_CLIENT; index++) {
                if ((strcmp(listOfClients[index].id, clientMsg.source) == 0) && (strcmp(listOfClients[index].password, clientMsg.data) == 0)) {
                    
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
                serverMsg.type = LO_ACK;
                serverMsg.size = 0;
                strcpy(serverMsg.source, clientMsg.source);
                strcpy(serverMsg.data, "Success");
                char type_string[5];
                char size_string[5];
                sprintf(type_string, "%d", serverMsg.type);
                sprintf(size_string, "%d", serverMsg.size);
                char *serv_message = struct_to_string(type_string, size_string, serverMsg.source, serverMsg.data);
                printf("User %d logged in. ", index);
                displayLoginStatus();
                write(ready_fd, serv_message, 1000);
                free(serv_message);
            }

            else if (flag == 0){
                serverMsg.type = LO_NAK;
                serverMsg.size = 100;
                strcpy(serverMsg.source, clientMsg.source);
                strcpy(serverMsg.data, "User id/password is incorrect, please retry.\n");
                char type_string[5];
                char size_string[5];
                sprintf(type_string, "%d", serverMsg.type);
                sprintf(size_string, "%d", serverMsg.size);
                char *serv_message = struct_to_string(type_string, size_string, serverMsg.source, serverMsg.data);
                write(ready_fd, serv_message, 1000);
                free(serv_message);
                close(ready_fd);
            }

            else{
                serverMsg.type = LO_NAK;
                serverMsg.size = 100;
                strcpy(serverMsg.source, clientMsg.source);
                strcpy(serverMsg.data, "This user has already logged, please try a different one.\n");
                char type_string[5];
                char size_string[5];
                sprintf(type_string, "%d", serverMsg.type);
                sprintf(size_string, "%d", serverMsg.size);
                char *serv_message = struct_to_string(type_string, size_string, serverMsg.source, serverMsg.data);
                write(ready_fd, serv_message, 1000);
                free(serv_message);
                close(ready_fd);
            }
        }

        else if (clientMsg.type == NEW_SESS){
            int client_index = atoi(clientMsg.source);

            for (int i = 0; i < NUM_CLIENT; i++){
                if (session_list[i] == NULL) {
                    session_list[i] = (char *)malloc(100);
                    strcpy(session_list[i], clientMsg.data);
                    break;
                }
            }
            
            displaySessionStatus(session_list);

            serverMsg.type = NS_ACK;
            serverMsg.size = 0;
            strcpy(serverMsg.source, clientMsg.source);
            strcpy(serverMsg.data, " ");

            char type_string[5];
            char size_string[5];
            sprintf(type_string, "%d", serverMsg.type);
            sprintf(size_string, "%d", serverMsg.size);
            char *serv_message = struct_to_string(type_string, size_string, serverMsg.source, serverMsg.data);
            write(ready_fd, serv_message, 1000);
            free(serv_message);
        }

        else if (clientMsg.type == JOIN){
            int client_index = atoi(clientMsg.source);
            int success = 0;

            for (int i = 0; i < NUM_CLIENT; i++){
                if (session_list[i] != NULL) {
                    if (strcmp(session_list[i], clientMsg.data) == 0){
                        listOfClients[client_index].session_id = i;
                        success = 1;
                        break;
                    }
                }
            }

            if (success) {
                printf("User %d joined %s. ", client_index, session_list[listOfClients[client_index].session_id]);
                displayUserSession(session_list);

                serverMsg.type = JN_ACK;
                serverMsg.size = 0;
                strcpy(serverMsg.source, clientMsg.source);
                strcpy(serverMsg.data, " ");
            }

            else{
                printf("User %d failed to join %s. ", client_index, clientMsg.data);
                displayUserSession(session_list);

                serverMsg.type = JN_NAK;
                serverMsg.size = 0;
                strcpy(serverMsg.source, clientMsg.source);
                strcpy(serverMsg.data, "Session does not exist.");
            }

            char type_string[5];
            char size_string[5];
            sprintf(type_string, "%d", serverMsg.type);
            sprintf(size_string, "%d", serverMsg.size);
            char *serv_message = struct_to_string(type_string, size_string, serverMsg.source, serverMsg.data);
            write(ready_fd, serv_message, 1000);
            free(serv_message);
        }
        
        else if (clientMsg.type == LEAVE_SESS){
            int client_index = atoi(clientMsg.source);

            printf("User %d left session %s. ", client_index, session_list[listOfClients[client_index].session_id]);
            listOfClients[client_index].session_id = -1;
            displayUserSession(session_list);

        }
        
        else if (clientMsg.type == QUERY){
            int client_index = atoi(clientMsg.source);

            serverMsg.type = QU_ACK;
            strcpy(serverMsg.source, clientMsg.source);
            
            memset(serverMsg.data, 0, MAX_DATA);

            for (int i = 0; i < NUM_CLIENT; i++){
                if (session_list[i] != NULL){
                    char temp[100];
                    sprintf(temp, "%s ", session_list[i]);
                    strcat(serverMsg.data, temp);
                }
            }

            strcat(serverMsg.data, "\n\t");

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
                    strcat(serverMsg.data, temp);
                }
            }

            serverMsg.size = strlen(serverMsg.data);

            char type_string[5];
            char size_string[5];
            sprintf(type_string, "%d", serverMsg.type);
            sprintf(size_string, "%d", serverMsg.size);
            char *serv_message = struct_to_string(type_string, size_string, serverMsg.source, serverMsg.data);
            write(ready_fd, serv_message, 1000);
            free(serv_message);
        }

        else if (clientMsg.type == MESSAGE){
            int client_index = atoi(clientMsg.source);
            int session_index = listOfClients[client_index].session_id;

            serverMsg.type = MESSAGE;
            strcpy(serverMsg.source, clientMsg.source);
            strcpy(serverMsg.data, clientMsg.data);

            serverMsg.size = strlen(serverMsg.data);

            char type_string[5];
            char size_string[5];
            sprintf(type_string, "%d", serverMsg.type);
            sprintf(size_string, "%d", serverMsg.size);

            char *serv_message = struct_to_string(type_string, size_string, serverMsg.source, serverMsg.data);

            for (int i = 0; i < NUM_CLIENT; i++){
                if (listOfClients[i].session_id == session_index){
                    write(listOfClients[i].accept_fd, serv_message, 1000);
                }
            }
            
            free(serv_message);
        }     

        else if (clientMsg.type == EXIT){
            int client_index = atoi(clientMsg.source);

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