#include "functions.h"

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

        listOfClients[i].loggedIn = false;
        listOfClients[i].acceptfd = 0;
        listOfClients[i].sessionId = -1;

        //Print client usernames and passwords on the screen
        printf("Client No.%d: %s, password: %s\n", i + 1, listOfClients[i].id, listOfClients[i].password);
    }

    //displayLoginStatus();

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

        //Updates the maxfd value
        for (int i = 0; i < NUM_CLIENT; i++){
            if (maxfd < listOfClients[i].acceptfd) {
                maxfd = listOfClients[i].acceptfd;
            }

            if (listOfClients[i].loggedIn)
                FD_SET(listOfClients[i].acceptfd, &listen_set);
        }

        select(maxfd + 1, &listen_set, NULL, NULL, NULL);

        int readyfd = sockfd;

        //Sets the readfd value to the first logged in clients' acceptfd
        for (int i = 0; i < NUM_CLIENT; i++){
            if (listOfClients[i].loggedIn) {
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

        char *inputPtr;
        inputPtr = strtok(buffer, ":");

        Message clientMsg;
        Message serverMsg;

        clientMsg.type = atoi(inputPtr);

        int idx = 1;

        //Getting the client message size, source, and data from the inputPtr
        while (inputPtr != NULL) {
            if (idx == 1) {
                inputPtr = strtok(NULL, ":");
                clientMsg.size = atoi(inputPtr);
                idx++;
            }

            else if (idx == 2) {
                inputPtr = strtok(NULL, ":");
                strcpy(clientMsg.source, inputPtr);
                idx++;
            }

            else if (idx == 3) {
                inputPtr = strtok(NULL, ":");
                strcpy(clientMsg.data, inputPtr);
                break;
            }
        }

        memset(buffer, 0, sizeof(buffer));

        //If the user uses a Login command
        if (clientMsg.type == LOGIN) {

            int flag = 0;
            int idx;

            //Check if the user exists in the listOfClients
            for (idx = 0; idx < NUM_CLIENT; idx++) {
                if ((strcmp(listOfClients[idx].id, clientMsg.source) == 0) && (strcmp(listOfClients[idx].password, clientMsg.data) == 0)) {
                    
                    if (listOfClients[idx].loggedIn) {
                        flag = -1;
                    }

                    else {
                        flag = 1;
                        listOfClients[idx].loggedIn = true;
                        listOfClients[idx].acceptfd = readyfd;
                    }

                    break;
                }
            }

            //Login is successful 
            if (flag == 1) {
                serverMsg.type = LO_ACK;
                serverMsg.size = 0;

                strcpy(serverMsg.source, clientMsg.source);
                strcpy(serverMsg.data, "Success");

                char stringType[5];
                char stringSize[5];

                sprintf(stringType, "%d", serverMsg.type);
                sprintf(stringSize, "%d", serverMsg.size);

                char *serv_message = struct_to_string(stringType, stringSize, serverMsg.source, serverMsg.data);

                printf("User %d logged in. ", idx);

                displayLoginStatus();

                write(readyfd, serv_message, 1000);
                free(serv_message);
            }

            //Login is unsuccessful
            else {
                serverMsg.type = LO_NAK;
                serverMsg.size = 100;

                strcpy(serverMsg.source, clientMsg.source);

                //Incorrect Username and Password
                if (flag == 0){
                    strcpy(serverMsg.data, "User id/password is incorrect, please retry.\n");
                }

                //If user enters credentials of a user that is already logged in
                else{
                    strcpy(serverMsg.data, "This user has already logged, please try a different one.\n");
                }
            
                char stringType[5];
                char stringSize[5];

                sprintf(stringType, "%d", serverMsg.type);
                sprintf(stringSize, "%d", serverMsg.size);

                char *serv_message = struct_to_string(stringType, stringSize, serverMsg.source, serverMsg.data);

                write(readyfd, serv_message, 1000);
                free(serv_message);
                close(readyfd);
            }
        }

        //If the user wants to start a new session
        else if (clientMsg.type == NEW_SESS){
            int clientIdx = atoi(clientMsg.source);

            //Find an empty session
            for (int i = 0; i < NUM_CLIENT; i++){
                if (listOfSessions[i] == NULL) {
                    listOfSessions[i] = (char *)malloc(100);
                    strcpy(listOfSessions[i], clientMsg.data);
                    break;
                }
            }
            
            displaySessionStatus(listOfSessions);

            //Send a New Session Acknowledgement
            serverMsg.type = NS_ACK;
            serverMsg.size = 0;

            strcpy(serverMsg.source, clientMsg.source);
            strcpy(serverMsg.data, " ");

            //Send a message acknowledging the creation of the session
            char stringType[5];
            char stringSize[5];

            sprintf(stringType, "%d", serverMsg.type);
            sprintf(stringSize, "%d", serverMsg.size);

            char *serv_message = struct_to_string(stringType, stringSize, serverMsg.source, serverMsg.data);

            write(readyfd, serv_message, 1000);
            free(serv_message);
        }

        //If the user wants to join an existing session
        else if (clientMsg.type == JOIN){
            int clientIdx = atoi(clientMsg.source);
            int success = 0;

            //Find the session in the list of sessions
            for (int i = 0; i < NUM_CLIENT; i++){
                if (listOfSessions[i] != NULL) {
                    if (strcmp(listOfSessions[i], clientMsg.data) == 0){
                        listOfClients[clientIdx].sessionId = i;
                        success = 1;
                        break;
                    }
                }
            }

            //If the session does exist, allow the user to join, and then send the acknowledgement of joining
            if (success) {
                printf("User %d joined %s. ", clientIdx, listOfSessions[listOfClients[clientIdx].sessionId]);
                displayUserSession(listOfSessions);

                serverMsg.type = JN_ACK;
                serverMsg.size = 0;

                strcpy(serverMsg.source, clientMsg.source);
                strcpy(serverMsg.data, " ");
            }

            //If the session wasn't found in the list of active sessions, let the user know that they were unsuccessful
            else{
                printf("User %d failed to join %s. ", clientIdx, clientMsg.data);
                displayUserSession(listOfSessions);

                serverMsg.type = JN_NAK;
                serverMsg.size = 0;

                strcpy(serverMsg.source, clientMsg.source);
                strcpy(serverMsg.data, "Session does not exist.");
            }

            //Send the message to the user
            char stringType[5];
            char stringSize[5];

            sprintf(stringType, "%d", serverMsg.type);
            sprintf(stringSize, "%d", serverMsg.size);

            char *serv_message = struct_to_string(stringType, stringSize, serverMsg.source, serverMsg.data);

            write(readyfd, serv_message, 1000);
            free(serv_message);
        }
        
        //If the user wants to leave the session
        else if (clientMsg.type == LEAVE_SESS){
            int clientIdx = atoi(clientMsg.source);

            int NumClients=0;

            //Display this message to the user
            printf("User %d left session %s. ", clientIdx, listOfSessions[listOfClients[clientIdx].sessionId]);
            
            
            for (int i = 0; i < NUM_CLIENT; i++){//find how many clients are in the session that we want to leave
                if(listOfClients[i].sessionId == listOfClients[clientIdx].sessionId){
                    NumClients++;
                }
            }

            if(NumClients == 1 ){//thers only 1 client in the session            
                listOfSessions[listOfClients[clientIdx].sessionId] = NULL;
               }
            
            listOfClients[clientIdx].sessionId = -1;
            displayUserSession(listOfSessions);
        }
        
        //If the user wants to get a list of all the online users and all the available sessions
        else if (clientMsg.type == QUERY){
            int clientIdx = atoi(clientMsg.source);

            serverMsg.type = QU_ACK;
            
            strcpy(serverMsg.source, clientMsg.source);
            memset(serverMsg.data, 0, MAX_DATA);

            //Get a list of all the active sessions
            for (int i = 0; i < NUM_CLIENT; i++){
                if (listOfSessions[i] != NULL){
                    char temp[100];
                    sprintf(temp, "%s ", listOfSessions[i]);
                    strcat(serverMsg.data, temp);
                }
            }

            strcat(serverMsg.data, "\n\t");

            //Get a list of all the logged in clients
            for (int i = 0; i < NUM_CLIENT; i++){
                if (listOfClients[i].loggedIn){
                    int sessionIdx = listOfClients[i].sessionId;

                    char temp[100];
                    char sess_name[20];

                    sprintf(temp, "%d:", i);
                    
                    //Also list whether each user is in a session or not
                    if (sessionIdx == -1) {
                        strcpy(sess_name, "Not in a session ");
                    }

                    else {
                        sprintf(sess_name, "%s ", listOfSessions[sessionIdx]);
                    }

                    strcat(temp, sess_name);
                    strcat(serverMsg.data, temp);
                }
            }

            serverMsg.size = strlen(serverMsg.data);

            //Send this message to the client
            char stringType[5];
            char stringSize[5];

            sprintf(stringType, "%d", serverMsg.type);
            sprintf(stringSize, "%d", serverMsg.size);

            char *serv_message = struct_to_string(stringType, stringSize, serverMsg.source, serverMsg.data);

            write(readyfd, serv_message, 1000);
            free(serv_message);
        }

        //If the user wants to send a message
        else if (clientMsg.type == MESSAGE){
            int clientIdx = atoi(clientMsg.source);
            int sessionIdx = listOfClients[clientIdx].sessionId;

            serverMsg.type = MESSAGE;

            strcpy(serverMsg.source, clientMsg.source);
            strcpy(serverMsg.data, clientMsg.data);

            serverMsg.size = strlen(serverMsg.data);

            char stringType[5];
            char stringSize[5];

            sprintf(stringType, "%d", serverMsg.type);
            sprintf(stringSize, "%d", serverMsg.size);

            char *serv_message = struct_to_string(stringType, stringSize, serverMsg.source, serverMsg.data);

            //If other users are in the same session as this user, send them the same message
            for (int i = 0; i < NUM_CLIENT; i++){
                if (listOfClients[i].sessionId == sessionIdx){
                    write(listOfClients[i].acceptfd, serv_message, 1000);
                }
            }
            
            free(serv_message);
        }     

        //If the user wants to leave the session
        else if (clientMsg.type == EXIT){
            int clientIdx = atoi(clientMsg.source);

            listOfClients[clientIdx].loggedIn = false;

            printf("User %d left the session. ", clientIdx);

            displayLoginStatus();

            //Mark user as logged our and reset everything about the user
            close(listOfClients[clientIdx].acceptfd);

            listOfClients[clientIdx].loggedIn = false;
            listOfClients[clientIdx].acceptfd = 0;
            listOfClients[clientIdx].sessionId = -1;
        }
    }

    return 0;
}