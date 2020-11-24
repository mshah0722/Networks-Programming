#ifndef FUNCTIONS_H
#define FUNCTIONS_H

#include <stdio.h>
#include <stdlib.h>
#include <libgen.h> 
#include <string.h>
#include <time.h> 
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

#define MAX_NAME 100
#define MAX_DATA 1000

typedef struct message {
    unsigned int type;
    unsigned int size;
    unsigned char source[MAX_NAME];
    unsigned char data[MAX_DATA];
}Message;

typedef enum message_type {
    LOGIN, // <client ID, password>
    LO_ACK,
    LO_NAK, // <reason for failure>
    EXIT,
    JOIN, // <session ID>
    JN_ACK, // <session ID>
    JN_NAK, // <session ID, reason for failure>
    LEAVE_SESS, 
    NEW_SESS, 
    NS_ACK, // <session ID>
    MESSAGE, // <message data>
    QUERY,
    QU_ACK // <users and session>
}Message_Type;

typedef struct id_pass {
    char id[100];
    char password[1000];
    int logged_in;
    int accept_fd;
    int session_id;
}ID_Pass;

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

#endif