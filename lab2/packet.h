#ifndef PACKET_H
#define PACKET_H

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


#define FRAGMENTSIZE 1000

struct packet {
    unsigned int total_frag;        
    unsigned int frag_no;        
    unsigned int size;          
    char* filename;                  
    char filedata[FRAGMENTSIZE];       
    struct packet * next;         
};

// //Prints all of the packet details (members of the packet struct)
// void printPacket(struct packet * p){
    // printf("Total Fragments: %d\n", p->total_frag);
    // printf("Fragment Number: %d\n", p->frag_no);
    // printf("Fragment Size: %d\n", p->size);
    // printf("File Name: %s\n", p->filename);
    // printf("File Data: %s\n", p->filedata);
// }

// //Prints all of the packets
// void printAllPackets(struct packet * p){
	// int i,total;
	// total = p-> 
    // while(p != NULL){
        // printPacket(p);
        // printf("\n");
        // p = p->next;
    // }
// }

//packet to string
char * compressPacket(struct packet * pack, int * length){
	
    //Finding size of string
    int string1 = snprintf(NULL, 0, "%d", pack->total_frag);
    int string2 = snprintf(NULL, 0, "%d", pack->frag_no);
    int string3 = snprintf(NULL, 0, "%d", pack->size);
    int string4 = strlen(pack->filename);
    int string5 = pack->size;
    int total_size = string1 + string2 + string3 + string4 + string5;
    
    
    char * compressedPacket = malloc((total_size+4)*sizeof(char));//create space for the string
    

    int index_offset = sprintf(compressedPacket, "%d:%d:%d:%s:",
                                pack->total_frag, pack->frag_no, pack->size, pack->filename);//store in string
    
     
    memcpy(&compressedPacket[index_offset], pack->filedata, pack->size);//Add the file data
    
    
    *length = index_offset + pack->size;//total length of the packet
    
    return compressedPacket;
}


//Create linked list of packets
struct packet * fileConvert(char * file_name){
    //Initializing vriables
    FILE * fp;
    struct packet * head, * prev;
    int num_bytes, total_fragments, frag_size, frag_num;
    char data[FRAGMENTSIZE];
    
    fp = fopen(file_name, "rb");//Open file in read-mode binary form
    

    fseek(fp, 0, SEEK_END);//sets cursor to end 
    num_bytes = ftell(fp);//gets the size of the file
    fseek(fp, 0, SEEK_SET);//sets the cursor to beginning
    
    //Determining the total number of fragments
    total_fragments = (num_bytes/FRAGMENTSIZE) + 1;
    
    //Create linked list of packets 
    for (frag_num=1; frag_num <= total_fragments; frag_num++) {
       
        struct packet * new_node = malloc(sizeof(struct packet));
        
		
        if(frag_num==1){//set root
            head = new_node;
        }
        else{//connect nodes
            prev->next = new_node;
        }
        
        frag_size = fread(data, 1, FRAGMENTSIZE, fp);//reads file and gets size
        
        //set values
        new_node->total_frag = total_fragments;
        new_node->frag_no = frag_num;
        new_node->size = frag_size;
        new_node->filename = file_name;
        
        memcpy(new_node->filedata, data, frag_size);//copying data

        new_node->next = NULL;//add null
        
        prev = new_node;//changing prev
    }
    
    fclose(fp);//close file
    
    return head;
}


//Converting the packet to its individual elements from string format
struct packet * extractPacket(char * packet_string){

    struct packet * separated_packet;
    char *total_frag_string, *frag_no_string, *size_string, *file_name, *data;
    int total_fragments, frag_num, frag_size;
    
    //splitting the string
    total_frag_string = strtok(packet_string, ":");
    frag_no_string = strtok(NULL, ":");
    size_string = strtok(NULL, ":");
    file_name = strtok(NULL, ":");
    
    //Converting the strings to int
    total_fragments = atoi(total_frag_string);
    frag_num = atoi(frag_no_string);
    frag_size = atoi(size_string);
    
    //index for data
    int index = strlen(total_frag_string) + strlen(frag_no_string) +
    strlen(size_string) + strlen(file_name) + 4;
    
    //copy data
    data = malloc(frag_size*sizeof(char));
    memcpy(data, &packet_string[index], frag_size);
    
    //asign values
    separated_packet = malloc(sizeof(struct packet));
    separated_packet->total_frag = total_fragments;
    separated_packet->frag_no = frag_num;
    separated_packet->size = frag_size;
    separated_packet->filename = file_name;
    memcpy(separated_packet->filedata, data, frag_size);
    
    
    if(frag_size < FRAGMENTSIZE){
        separated_packet->filedata[frag_size] = '\0';//adding null character
    }
    return separated_packet;
}


//delete linked list
void freePackets(struct packet * head){
    
    struct packet * curr = head;
    struct packet * next;
    
    //looop till end
    while(curr != NULL) {
       
        next = curr->next;
        
        free(curr);//delete
        
        curr = next;//set next
    }
}


#endif