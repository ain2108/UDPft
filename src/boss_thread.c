#include "boss_thread.h"

pthread_mutex_t boss_lock;


// Function reads MSS bytes from offset, puts them into the buffer. 
int extractData(char * file_name, int offset, char * buffer){

  fprintf(stderr, "Extracting  %s...\n", file_name);  
  memset(buffer, 0 , MSS);
  FILE * fp = fopen(file_name, "rb");
  if(fp == NULL) die("fopen() failed:");
  fseek(fp, offset, SEEK_SET);

  // If read less than expected 
  int bytesRead;
  if( (bytesRead = fread(buffer, 1, MSS, fp)) != MSS){
    // If serious I/O error
    if(ferror(fp)){
      fclose(fp);
      die("fread() failed: ");
    }
    // If EOF
    fclose(fp);
    fprintf(stderr, "%d\n", bytesRead);
    return bytesRead;
  }
  
  fclose(fp);
  return bytesRead;
}

// Function builds a complete packet, readu to be sent. 
Packet * buildPacket(char * file_name, unsigned short sport,
		unsigned short dport, unsigned int seq_num){

  // HEAP allocation
  Packet * pack = (Packet * ) malloc(sizeof(Packet));
  // char * data 
  memset((char *) pack, 0, sizeof(Packet));
  // memset(data, 0, MSS * sizeof(char));

  // Populating the header
  strncpy(pack->header.sport, (const char *) &sport, 2);
  strncpy(pack->header.dport, (const char *) &dport, 2);
  strncpy(pack->header.seq_num, (const char *) &seq_num, 4);

  int fin = 0;
  // Reading the data from file into the Packet
  int bytesRead = extractData(file_name, seq_num, pack->data);
  if(bytesRead != MSS){
    fin = 1;
  }

  // Set the fin bit
  if(fin) pack->header.fun_stuff[1] |= 1 << 0;
  // Set the header length. No options => 20 bytes long => 5 => 0101 0000
  pack->header.fun_stuff[0] |= 1 << 4;
  pack->header.fun_stuff[0] |= 1 << 6;
  // remmeber the number of bytes read as data
  pack->data_size = bytesRead;
  // Calculate the checksum and add it to the packet.
  unsigned short checkSum =  calculateChecksum(pack);
  strncpy(pack->header.inet_checksum, (const char *) &checkSum, 2);   

  printPacketHeader(pack);
  
  return pack;
}

// Calculate the checksum
unsigned short calculateChecksum(Packet * pack){

  unsigned short checkSum = 0;
  char * temp = (char *)pack;

  // Checksum of the header
  int i = 0;
  for(i = 0; i < (MSS/2) + 10; i++){
    checkSum += *((unsigned short *) &temp[2*i]);
  }

  return ~checkSum;
}

void printPacketHeader(Packet * pack){

  unsigned short sport = *((unsigned short *)pack->header.sport);
  unsigned short dport = *((unsigned short *)pack->header.dport);
  unsigned int seq_num = *((int *)pack->header.seq_num);
  unsigned char fin = *((unsigned char *) &pack->header.fun_stuff[1]);
  unsigned char length = *((unsigned char *) &pack->header.fun_stuff[0]); 
  unsigned short checkSum = *((unsigned short *)pack->header.inet_checksum);
  length = length >> 4;
  // Printing it
  fprintf(stderr, "Printing packet header...\n");
  fprintf(stderr, "Source Port: %d\n", sport);
  fprintf(stderr, "Destination Port: %d\n", dport);
  fprintf(stderr, "Sequence Num: %d\n", seq_num);
  fprintf(stderr, "Header length: %d\n", length);
  fprintf(stderr, "FIN: %d\n", fin);
  fprintf(stderr, "Checksum: %d\n", checkSum);
  char temp[MSS + 1];
  memset(temp, 0, MSS + 1);
  strncpy(temp, pack->data, pack->data_size);
  strcat(temp, "\0");
  fprintf(stderr, "===DATA===:\n%s\n===END===\n", temp);
  fprintf(stderr, "DATA LENGTH: %d\n", pack->data_size);
  fprintf(stderr, "Done.\n");
  return;
}
 

void initPacketStatusDB(PacketStatus * PSDB, int size){
  memset((char *) PSDB, 1, size * sizeof(Packet));
  int i;
  for(i = 0; i < size; i++){
    PSDB[i].seq_num = i;
    PSDB[i].sent = 0;
    PSDB[i].acked = 0;
  }
  return;
}



int boss_thread_function(PacketStatus * PSDB, int window_size, int ack_port_num){
  // Start listening on the socket for ACKS
  // When receive an Packet, look inside.
  // Fetch the seq number
  // Lock
  // Access the DB and change acked=1
  // Unlock
  return 0;
}

