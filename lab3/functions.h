#ifndef FUNCTIONS_H
#define FUNCTIONS_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <errno.h>
#include <time.h>
#include <stdbool.h>
#include <math.h>


#define MAXFRAGLEN 1000

struct packet {
    unsigned int total_frag;        
    unsigned int frag_no;        
    unsigned int size;          
    char* filename;                  
    char filedata[MAXFRAGLEN];       
    struct packet * next;         
};


//Packet to string
char * struct_to_string(struct packet * pack, int * length){
	
    //Finding size of string
    int string1 = snprintf(NULL, 0, "%d", pack->total_frag);
    int string2 = snprintf(NULL, 0, "%d", pack->frag_no);
    int string3 = snprintf(NULL, 0, "%d", pack->size);
    int string4 = strlen(pack->filename);
    int string5 = pack->size;
    int total_size = string1 + string2 + string3 + string4 + string5;
    
    //Create space for the string
    char * compressedPacket = malloc((total_size+4)*sizeof(char));
    
    //Store in string
    int index_offset = sprintf(compressedPacket, "%d:%d:%d:%s:",
                                pack->total_frag, pack->frag_no, pack->size, pack->filename);
    
    //Add the file data
    memcpy(&compressedPacket[index_offset], pack->filedata, pack->size);
    
    //Total length of the packet
    *length = index_offset + pack->size;
    
    return compressedPacket;
}


//Create linked list of packets
struct packet * create_linked_packets(char * file_name){
    //Initializing vriables
    FILE * fp;
    struct packet * head_packet, * prev;
    int num_bytes, total_fragments, frag_size, frag_num;
    char data[MAXFRAGLEN];
    
    fp = fopen(file_name, "rb");//Open file in read-mode binary form
    

    fseek(fp, 0, SEEK_END);//Sets cursor to end 
    num_bytes = ftell(fp);//Gets the size of the file
    fseek(fp, 0, SEEK_SET);//Sets the cursor to beginning
    
    //Determining the total number of fragments
    total_fragments = (num_bytes/MAXFRAGLEN) + 1;
    
    //Create linked list of packets 
    for (frag_num=1; frag_num <= total_fragments; frag_num++) {
       
        struct packet * new_node = malloc(sizeof(struct packet));
        
		//Set root
        if(frag_num==1){
            head_packet = new_node;
        }

        //Connect nodes
        else{
            prev->next = new_node;
        }
        
        //Reads file and gets size
        frag_size = fread(data, 1, MAXFRAGLEN, fp);
        
        //set values
        new_node->total_frag = total_fragments;
        new_node->frag_no = frag_num;
        new_node->size = frag_size;
        new_node->filename = file_name;
        
        //Copying data
        memcpy(new_node->filedata, data, frag_size);

        //Add null
        new_node->next = NULL;
        
        //Changing prev
        prev = new_node;
    }
    
    //Close file
    fclose(fp);
    
    return head_packet;
}


//Converting the packet to its individual elements from string format
struct packet * string_to_struct(char * packet_string){

    struct packet * separated_packet;
    char *total_frag_string, *frag_no_string, *size_string, *file_name, *data;
    int total_fragments, frag_num, frag_size;
    
    //Splitting the string
    total_frag_string = strtok(packet_string, ":");
    frag_no_string = strtok(NULL, ":");
    size_string = strtok(NULL, ":");
    file_name = strtok(NULL, ":");
    
    //Converting the strings to int
    total_fragments = atoi(total_frag_string);
    frag_num = atoi(frag_no_string);
    frag_size = atoi(size_string);
    
    //Index for data
    int index = strlen(total_frag_string) + strlen(frag_no_string) +
    strlen(size_string) + strlen(file_name) + 4;
    
    //Copy data
    data = malloc(frag_size*sizeof(char));
    memcpy(data, &packet_string[index], frag_size);
    
    //Assign values
    separated_packet = malloc(sizeof(struct packet));
    separated_packet->total_frag = total_fragments;
    separated_packet->frag_no = frag_num;
    separated_packet->size = frag_size;
    separated_packet->filename = file_name;
    memcpy(separated_packet->filedata, data, frag_size);
    
    
    if(frag_size < MAXFRAGLEN){
        //Adding null character
        separated_packet->filedata[frag_size] = '\0';
    }
    return separated_packet;
}

#endif
