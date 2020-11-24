#include "data.h"
#define PORT 3000 
#define MAXLEN 1024

bool find_client(char* clientID_str, char* password_str);

int main(int argc, char const *argv[]) { 
    int port;
    switch(argc) {
        case 2:
            port = atoi(argv[1]);
            break;
        default:
            fprintf(stderr, "Use the format: ./server <Port Number>\n");
            exit(0);
    }
    
    int server_socket, client_socket, read_value; 
    struct sockaddr_in server_address; 
    int addrlen = sizeof(server_address); 
    int option = 1;
    char message_buffer[MAXLEN] = {0};  
    int command = 0;
    struct account_info* account_list = NULL;
    struct session* session_list = NULL;
       
    // Creating socket file descriptor 
    if ((server_socket = socket(AF_INET, SOCK_STREAM, 0)) == 0) 
    { 
        perror("socket failed"); 
        exit(EXIT_FAILURE); 
    } 
    setsockopt(server_socket, SOL_SOCKET, SO_REUSEADDR, &option, sizeof(option));
    
    server_address.sin_family = AF_INET; 
    server_address.sin_addr.s_addr = INADDR_ANY; 
    server_address.sin_port = htons(port); 
    
    if (bind(server_socket, (struct sockaddr *)&server_address, sizeof(server_address)) < 0) { 
        perror("bind failed"); 
        exit(EXIT_FAILURE); 
    } 

    if (listen(server_socket, 7) < 0) { 
        perror("listen"); 
        exit(EXIT_FAILURE); 
    } 

    if ((client_socket = accept(server_socket, (struct sockaddr *)&server_address, (socklen_t*)&addrlen)) < 0) { 
        perror("accept"); 
        exit(EXIT_FAILURE); 
    } 

    while(command != EXIT){
        read(client_socket , message_buffer, MAXLEN); 
        struct packet
        * currentPacket = extractPacket(message_buffer);
        printPacket(currentPacket);
        command = currentPacket->type;

        if (command == LOGIN) {
            char temp_buffer[MAXLEN];
            strcpy(temp_buffer, currentPacket->data);
                        
            char username[MAXLEN], password[MAXLEN];
            sscanf(temp_buffer, "%[^,],%s", username, password);
            if (find_client(username, password)) {
                //Add the client into a data structure
                struct account_info* new_account = create_account(username, password);
                account_list = add_to_account_list(account_list, new_account);
                //print_account_info(account_list);

                struct packet* pack = malloc(sizeof(struct packet));
                pack->type = LO_ACK;
                strcpy(pack->source, "Server");
                strcpy(pack->data, "0");
                pack->size = 0;

                send(client_socket , compressPacket(pack) , strlen(compressPacket(pack)) , 0 ); 

                //send(client_socket, "LO_ACK", strlen("LO_ACK"), 0); 
            }
            else {
                struct packet* pack = malloc(sizeof(struct packet));
                pack->type = LO_NAK;
                strcpy(pack->source, "Server");
                strcpy(pack->data, "0");
                pack->size = 0;
                send(client_socket , compressPacket(pack) , strlen(compressPacket(pack)) , 0 ); 
                // send(client_socket, "LO_NAK", strlen("LO_NAK"), 0); 
            }
        }
        else if (command == EXIT){
            send(client_socket, "EXIT", strlen("EXIT"), 0);
            break;
        }
        else if (command == NEW_SESS) {
            struct session* new_session = create_session(currentPacket->data);
            session_list = add_to_session_list(session_list, new_session);
            struct account_info* new_account = search_account(account_list, currentPacket->source);
            new_account->connected = true;
            session_list = add_account_to_session(session_list, new_account, currentPacket->data);
            //print_session_info(session_list);
            
            struct packet* pack = malloc(sizeof(struct packet));
            pack->type = NS_ACK;
            strcpy(pack->source, "Server");
            strcpy(pack->data, currentPacket->data);
            pack->size = strlen(currentPacket->data);
            send(client_socket , compressPacket(pack) , strlen(compressPacket(pack)) , 0 ); 

            //send(client_socket, "NS_ACK", strlen("NS_ACK"), 0);
        }
        else if (command == QUERY) {
            char list[MAXLEN];
            if ((account_list != NULL) && (session_list != NULL)) {
                strcat(list,"Active Users\n");
                struct account_info* current_account = account_list;

                while (current_account != NULL){
                    if (current_account->connected) {
                        strcat(list,current_account->clientID);
                        strcat(list,"\n");
                    }
                    current_account = current_account->next_account;
                }

                strcat(list,"\nActive Sessions\n"); 
                struct session* current_session = session_list;

                while (current_session != NULL){
                    if (current_session->active) {
                        strcat(list,current_session->session_id);
                        strcat(list,"\n");
                    }
                    current_session = current_session->next_session;
                }
            }
            else {
                strcat(list,"No active users or sessions.\n");
            }

            struct packet* pack = malloc(sizeof(struct packet));
            pack->type = QU_ACK;
            strcpy(pack->source, "Server");
            strcpy(pack->data, list);
            pack->size = strlen(pack->data);
            send(client_socket , compressPacket(pack) , strlen(compressPacket(pack)) , 0 ); 
            memset(list, 0, MAXLEN);

            //send(client_socket, "NS_ACK", strlen("NS_ACK"), 0);
        }
    }
    close(server_socket);
    return 0; 
} 

//Searching for a client in the database
bool find_client(char* clientID_str, char* password_str) {
    char* file_name = "userData.txt";
    char* file_buffer[MAXLEN];
    FILE* file_pointer = fopen(file_name, "r");
    while (fscanf(file_pointer,"%s",file_buffer) == 1) {
        if (strcmp(file_buffer, clientID_str) == 0) {
            if (fscanf(file_pointer,"%s\n",file_buffer) == 1) {
                char password_copy[MAXLEN];
                strcpy(password_copy, password_str);
                strncat(password_copy, "\\", 1);
                if (strcmp(file_buffer, password_copy) == 0) {
                    return true;
                }
            }
        }
    }
    return false;
}