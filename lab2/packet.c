#include "packet.h"

//Prints all of the packet details (members of the packet struct)
void printPacket(struct packet * p){
    printf("Total Fragments: %d\n", p->totalFragments);
    printf("Fragment Number: %d\n", p->fragmentNumber);
    printf("Fragment Size: %d\n", p->fragmentSize);
    printf("File Name: %s\n", p->fileName);
    printf("File Data: %s\n\n", p->fileData);
}

//Prints all of the packets
void printAllPackets(struct packet * p){
	int i, total;
	total = p->totalFragments;
	for(i=0; i < total; i++)
	{
		printPacket(p);
		p = p->nextPacket;
    }
}

//FILE *fopen( const char * filename, const char * mode );

struct packet * fileConvert(char * filename){
    //Initializing vriables
    FILE * filePointer;
	
    struct packet * root_packet, * prev_packet;
    int num_bytes, total_frag, size, frag_no;
    char data[FRAGMENTSIZE];
    
    
    //Opens the file in read-mode and binary form
    filePointer = fopen(filename, "rb");
    
    //Determining total number of bytes in the file
    //By reading the file entirely, then going back to the start
    fseek(filePointer, 0, SEEK_END);
    num_bytes = ftell(filePointer);
    fseek(filePointer, 0, SEEK_SET);
    
    //Determining the total number of fragments
    total_frag = (num_bytes/FRAGMENTSIZE) + 1;
    
    //Creating the linked list of packets so they are connected to each other
    for (frag_no=1; frag_no<=total_frag; frag_no++) {
        //Allocating memory for a new packet
        struct packet * new_packet = malloc(sizeof(struct packet));
        
        //Identifying the root packet (or first packet in the list)
        if(frag_no==1){
            root_packet = new_packet;
        }
        else{
            prev_packet->nextPacket = new_packet;
        }
        
        //Obtain the bitwise data in the file
        size = fread(data, 1, FRAGMENTSIZE, filePointer);
        
        //Entering all of the packet details
        new_packet->totalFragments = total_frag;
        new_packet->fragmentNumber = frag_no;
        new_packet->fragmentSize = size;
        new_packet->fileName = filename;
        //Using memcpy to copy the block of memory
        memcpy(new_packet->fileData, data, size);
        //Adding a null at the end of the linked list
        new_packet->nextPacket = NULL;
        
        //Setting a pointer to the previous packet so the nextPacket member will have a value
        prev_packet = new_packet;
    }
    
    //Closing the file
    fclose(filePointer);
    
    //Returning the head of the linked list
    return root_packet;
};

//Frees the memory that was used for the linked list
void freePackets(struct packet * root){
    //Starting at the root of the linked list
    struct packet * curr_packet = root;
    struct packet * next_packet;
    
    //Iterating until reaches the end of the linked list
    while(curr_packet != NULL) {
        //Sets pointer to the next packet
        next_packet = curr_packet->nextPacket;
        
        //Frees the memory for the current packet
        free(curr_packet);
        
        //Sets the new current packet as the next packet
        curr_packet = next_packet;
    }
}

//Converting the packet to a string format for sending
char * compressPacket(struct packet * pack, int * len){
    //Finding the size of each element to allocate spaace
    int s1 = snprintf(NULL, 0, "%d", pack->totalFragments);
    int s2 = snprintf(NULL, 0, "%d", pack->fragmentNumber);
    int s3 = snprintf(NULL, 0, "%d", pack->fragmentSize);
    int s4 = strlen(pack->fileName);
    int s5 = pack->fragmentSize;
    int total_size = s1+s2+s3+s4+s5;
    
    //Allocating space for the string to be stored
    char * compressedPacket = malloc((total_size+4)*sizeof(char));
    
    //Converting and storing the packet details into the string
    int header_offset = sprintf(compressedPacket, "%d:%d:%d:%s:",
                                pack->totalFragments, pack->fragmentNumber, pack->fragmentSize, pack->fileName);
    
    //Adding the file data into the packet in the string form
    memcpy(&compressedPacket[header_offset], pack->fileData, pack->fragmentSize);
    
    //Finding the total length of the packet
    *len = header_offset + pack->fragmentSize;
    
    return compressedPacket;
}

//Converting the packet to its individual elements from string format
struct packet * extractPacket(char * packet_str){
    //Declaring variables
    struct packet * extractedPacket;
    char *total_frag_str, *frag_no_str, *size_str, *filename, *filedata;
    int total_frag, frag_no, size;
    
    //Splitting the string based on the the : delimeter
    total_frag_str = strtok(packet_str, ":");
    frag_no_str = strtok(NULL, ":");
    size_str = strtok(NULL, ":");
    filename = strtok(NULL, ":");
    
    //Converting the strings to int
    total_frag = atoi(total_frag_str);
    frag_no = atoi(frag_no_str);
    size = atoi(size_str);
    
    //Obtaining the size for the UDP header
    int udp_header_size = strlen(total_frag_str) + strlen(frag_no_str) +
    strlen(size_str) + strlen(filename) + 4;
    
    //Transferring the file data from the string format into bits
    filedata = malloc(size*sizeof(char));
    memcpy(filedata, &packet_str[udp_header_size], size);
    
    //Setting all of the packet details to its corresponding members
    extractedPacket = malloc(sizeof(struct packet));
    extractedPacket->totalFragments = total_frag;
    extractedPacket->fragmentNumber = frag_no;
    extractedPacket->fragmentSize = size;
    extractedPacket->fileName = filename;
    memcpy(extractedPacket->fileData, filedata, size);
    
    //Adding the Null character to the data
    if(size < FRAGMENTSIZE){
        extractedPacket->fileData[size] = '\0';
    }
    return extractedPacket;
}
