#ifndef FUNCTIONS_H
#define FUNCTIONS_H

#include <stdio.h>
#include <stdlib.h>
#include <libgen.h> 
#include <string.h>
#include <time.h> 
#include <stdbool.h>
#include <sys/types.h> 
#include <sys/socket.h> 
#include <sys/stat.h>
#include <sys/time.h>
#include <netinet/in.h> 
#include <unistd.h>
#include <arpa/inet.h> 
#include <netdb.h>
#include <error.h>
#include <errno.h>
#include <ctype.h>
#include <fcntl.h>
#include <dirent.h>

#define NUM_CLIENT 5
#define MAX_NAME 100
#define MAX_DATA 1000

typedef struct message {
    unsigned int type;
    unsigned int size;
    unsigned char source[MAX_NAME];
    unsigned char data[MAX_DATA];
}Message;

typedef enum message_type {
    LOGIN, // <client ID, password>, Login with the server
    LO_ACK, // Acknowledge successful login
    LO_NAK, // <reason for failure>, Negative acknowledgement of login
    EXIT, // Exit from server
    JOIN, // <session ID>, Join a conference session
    JN_ACK, // <session ID>, Acknowledge successful conference session join
    JN_NAK, // <session ID, reason for failure>, Negative acknowledgement of joining the session
    LEAVE_SESS, // Leave a conference session
    NEW_SESS, // Create new conference session
    NS_ACK, // <session ID>, Acknowledge new conference session
    MESSAGE, // <message data>, Send a message to the session if it is received
    QUERY, // Get a list of online users and available sessions
    QU_ACK // <users and session>, Reply followed by a list of users online
}Message_Type;

typedef struct user_info {
    char id[100];
    char password[1000];
    bool loggedIn;
    int acceptfd;
    int sessionId;
}USER_INFO;

char* struct_to_string (char *string1, char *string2, char *string3, char *string4) {

	int string_length = strlen(string1) + strlen(string2) + strlen(string3) + strlen(string4) + 4;
	char* string = (char *)malloc(2000 * sizeof(char));

	int i = 0;

	while (string1[i] != '\0') {
		string[i] = string1[i];
		i++;
	}

	string[i] = ':';
	i++;
	int j = 0;
	while (string2[j] != '\0') {
		string[i+j] = string2[j];
		j++;
	}

    string[i+j] = ':';
	j++;
	int k = 0;
	while (string3[k] != '\0') {
		string[i+j+k] = string3[k];
		k++;
	}

    string[i+j+k] = ':';
	k++;
	int l = 0;
	while (string4[l] != '\0') {
		string[i+j+k+l] = string4[l];
		l++;
	}

    string[i+j+k+l] = '\0';

	return string;
}

void displayLoginStatus() {

    printf("User status:");

    for (int i = 0; i < NUM_CLIENT; i++) {
        printf(" %d->", i);
        if (client_list[i].logged_in) printf("on");
        else printf("off");
    }

    printf("\n");
}

void displayUserSession (char **session_list) {

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

void displaySessionStatus (char **session_list) {

    printf("Session status:");

    for (int i = 0; i < NUM_CLIENT; i++){
        if (session_list[i] != NULL){
            printf(" %d->%s", i, session_list[i]);
        }
    }

    printf("\n");
}

#endif