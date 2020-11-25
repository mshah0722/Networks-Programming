#include "functions.h"

//List of client access
USER_INFO listOfClients[NUM_CLIENT];

int main (int argc, char *argv[]){

    int port;

    //Check if you don't have two arguments
    if (argc != 2){
        fprintf(stderr, "Incorrect argument amount. Usage: server <udp listen port>\n");
        exit(1);
    }

    //Set port number from the input
    else {
        port = atoi(argv[1]);
    }

    // Setting the credentials for each client
    for (int i = 0; i < NUM_CLIENT; i++) {
        strcpy(listOfClients[i].id, "User_X");
        listOfClients[i].id[5] = i + '0';
        char password[4];
        sprintf(password, "%d%d%d%d", i, i, i, i);
        strcpy(listOfClients[i].password, password);
        listOfClients[i].logged_in = 0;
        listOfClients[i].acceptfd = 0;
        listOfClients[i].session_id = -1;

        //Print client usernames and passwords on the screen
        printf("Client No.%d: %s, password: %s\n", i + 1, listOfClients[i].id, listOfClients[i].password);
    }

    displayLoginStatus();

    //Initialize all the sessions as Null
    char *listOfSessions[NUM_CLIENT];
    for (int i = 0; i < NUM_CLIENT; i++) {
        listOfSessions[i] = NULL;
    }
    
    //Check if a TCP socket can be created
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);

    if (sockfd < 0){
        fprintf(stderr, "socket error\n");
    }

    //Dynamicially create space for server address and client address
    struct sockaddr_in *server_addr, *client_addr;

    server_addr = (struct sockaddr_in *)malloc(sizeof(struct sockaddr_in));
    client_addr = (struct sockaddr_in *)malloc(sizeof(struct sockaddr_in));

    memset(server_addr, 0, sizeof(struct sockaddr_in));
    memset(client_addr, 0, sizeof(struct sockaddr_in));

    //Create a TCP socket
    server_addr->sin_addr.s_addr = INADDR_ANY;
    server_addr->sin_port = htons(port);
    server_addr->sin_family = AF_INET;
    unsigned int server_size = sizeof(struct sockaddr);

    int bindfd = bind(sockfd, (const struct sockaddr*)server_addr, sizeof(struct sockaddr));

    //Assigning a name binding to the socket 
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

    while (true) {

        FD_ZERO(&listen_set);

        FD_SET(sockfd, &listen_set);

        int maxfd = sockfd;

        //
        for (int i = 0; i < NUM_CLIENT; i++){
            if (maxfd < listOfClients[i].acceptfd) maxfd = listOfClients[i].acceptfd;

            if (listOfClients[i].logged_in != 0)
                FD_SET(listOfClients[i].acceptfd, &listen_set);
        }

        select(maxfd + 1, &listen_set, NULL, NULL, NULL);

        //
        int readyfd = sockfd;
        for (int i = 0; i < NUM_CLIENT; i++){
            if (listOfClients[i].logged_in == 1) {
                if (FD_ISSET(listOfClients[i].acceptfd, &listen_set)) {
                    readyfd = listOfClients[i].acceptfd;
                    break;
                }
            }
        }

        char buffer[1000];
        
        memset(buffer, 0, sizeof(buffer));

        //If there's a new login request
        if (readyfd == sockfd) {
            readyfd = accept(sockfd, (struct sockaddr*)client_addr, &server_size);
            if (readyfd < 0) {
                fprintf(stderr, "server accept error");
                exit(0);
            }
        }
            
        read(readyfd, buffer, 1000);
        
        printf("Message received from client ---> %s\n", buffer);

        char *inputPtr;
        inputPtr = strtok(buffer, ":");

        Message clientMsg;
        Message serverMsg;

        clientMsg.type = atoi(inputPtr);

        int idx = 1;

        while (inputPtr != NULL) {
            if (idx == 1) { // size
                inputPtr = strtok(NULL, ":");
                clientMsg.size = atoi(inputPtr);
                idx++;
            }

            else if (idx == 2) { // source
                inputPtr = strtok(NULL, ":");
                strcpy(clientMsg.source, inputPtr);
                idx++;
            }

            else if (idx == 3) { // data
                inputPtr = strtok(NULL, ":");
                strcpy(clientMsg.data, inputPtr);
                break;
            }
        }

        memset(buffer, 0, sizeof(buffer));

        if (clientMsg.type == LOGIN) {

            int flag = 0;
            int idx;

            for (idx = 0; idx < NUM_CLIENT; idx++) {
                if ((strcmp(listOfClients[idx].id, clientMsg.source) == 0) && (strcmp(listOfClients[idx].password, clientMsg.data) == 0)) {
                    
                    if (listOfClients[idx].logged_in) {
                        flag = -1;
                    }

                    else {
                        flag = 1;
                        listOfClients[idx].logged_in = 1;
                        listOfClients[idx].acceptfd = readyfd;
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
                printf("User %d logged in. ", idx);
                displayLoginStatus();
                write(readyfd, serv_message, 1000);
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
                write(readyfd, serv_message, 1000);
                free(serv_message);
                close(readyfd);
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
                write(readyfd, serv_message, 1000);
                free(serv_message);
                close(readyfd);
            }
        }

        else if (clientMsg.type == NEW_SESS){
            int client_idx = atoi(clientMsg.source);

            for (int i = 0; i < NUM_CLIENT; i++){
                if (listOfSessions[i] == NULL) {
                    listOfSessions[i] = (char *)malloc(100);
                    strcpy(listOfSessions[i], clientMsg.data);
                    break;
                }
            }
            
            displaySessionStatus(listOfSessions);

            serverMsg.type = NS_ACK;
            serverMsg.size = 0;
            strcpy(serverMsg.source, clientMsg.source);
            strcpy(serverMsg.data, " ");

            char type_string[5];
            char size_string[5];
            sprintf(type_string, "%d", serverMsg.type);
            sprintf(size_string, "%d", serverMsg.size);
            char *serv_message = struct_to_string(type_string, size_string, serverMsg.source, serverMsg.data);
            write(readyfd, serv_message, 1000);
            free(serv_message);
        }

        else if (clientMsg.type == JOIN){
            int client_idx = atoi(clientMsg.source);
            int success = 0;

            for (int i = 0; i < NUM_CLIENT; i++){
                if (listOfSessions[i] != NULL) {
                    if (strcmp(listOfSessions[i], clientMsg.data) == 0){
                        listOfClients[client_idx].session_id = i;
                        success = 1;
                        break;
                    }
                }
            }

            if (success) {
                printf("User %d joined %s. ", client_idx, listOfSessions[listOfClients[client_idx].session_id]);
                displayUserSession(listOfSessions);

                serverMsg.type = JN_ACK;
                serverMsg.size = 0;
                strcpy(serverMsg.source, clientMsg.source);
                strcpy(serverMsg.data, " ");
            }

            else{
                printf("User %d failed to join %s. ", client_idx, clientMsg.data);
                displayUserSession(listOfSessions);

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
            write(readyfd, serv_message, 1000);
            free(serv_message);
        }
        
        else if (clientMsg.type == LEAVE_SESS){
            int client_idx = atoi(clientMsg.source);

            printf("User %d left session %s. ", client_idx, listOfSessions[listOfClients[client_idx].session_id]);
            listOfClients[client_idx].session_id = -1;
            displayUserSession(listOfSessions);

        }
        
        else if (clientMsg.type == QUERY){
            int client_idx = atoi(clientMsg.source);

            serverMsg.type = QU_ACK;
            strcpy(serverMsg.source, clientMsg.source);
            
            memset(serverMsg.data, 0, MAX_DATA);

            for (int i = 0; i < NUM_CLIENT; i++){
                if (listOfSessions[i] != NULL){
                    char temp[100];
                    sprintf(temp, "%s ", listOfSessions[i]);
                    strcat(serverMsg.data, temp);
                }
            }

            strcat(serverMsg.data, "\n\t");

            for (int i = 0; i < NUM_CLIENT; i++){
                if (listOfClients[i].logged_in != 0){
                    int session_idx = listOfClients[i].session_id;
                    char temp[100];
                    char sess_name[20];
                    sprintf(temp, "%d->", i);
                    
                    if (session_idx == -1) {
                        strcpy(sess_name, "N/A ");
                    }

                    else {
                        sprintf(sess_name, "%s ", listOfSessions[session_idx]);
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
            write(readyfd, serv_message, 1000);
            free(serv_message);
        }

        else if (clientMsg.type == MESSAGE){
            int client_idx = atoi(clientMsg.source);
            int session_idx = listOfClients[client_idx].session_id;

            serverMsg.type = MESSAGE;
            strcpy(serverMsg.source, clientMsg.source);
            strcpy(serverMsg.data, clientMsg.data);

            serverMsg.size = strlen(serverMsg.data);

            char type_string[5];
            char size_string[5];
            sprintf(type_string, "%d", serverMsg.type);
            sprintf(size_string, "%d", serverMsg.size);

            char *serv_message = struct_to_string(type_string, size_string, serverMsg.source, serverMsg.data);

            //
            for (int i = 0; i < NUM_CLIENT; i++){
                if (listOfClients[i].session_id == session_idx){
                    write(listOfClients[i].acceptfd, serv_message, 1000);
                }
            }
            
            free(serv_message);
        }     

        //
        else if (clientMsg.type == EXIT){
            int client_idx = atoi(clientMsg.source);

            listOfClients[client_idx].logged_in = 0;
            printf("User %d quitted. ", client_idx);
            displayLoginStatus();

            close(listOfClients[client_idx].acceptfd);
            listOfClients[client_idx].logged_in = 0;
            listOfClients[client_idx].acceptfd = 0;
            listOfClients[client_idx].session_id = -1;
        }
    }

    return 0;
}