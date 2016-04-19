#include "packet.h"

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

  // int fin = 0;
  // Reading the data from file into the Packet
  int bytesRead = extractData(file_name, seq_num, pack->data);
  //if(bytesRead != MSS){
  //  fin = 1;
  //}

  // Set the fin bit
  // if(fin) pack->header.fun_stuff[1] |= 1 << 0;
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
  unsigned int ack_num = *((int *)pack->header.ack_num);
  length = length >> 4;
  // Printing it
  fprintf(stderr, "Printing packet header...\n");
  fprintf(stderr, "Source Port: %d\n", sport);
  fprintf(stderr, "Destination Port: %d\n", dport);
  fprintf(stderr, "Sequence Num: %d\n", seq_num);
  fprintf(stderr, "Ack Num: %d\n", ack_num); 
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

// Initiate the status DB
void initPacketStatusDB(PacketStatus * PSDB, int window_size){
  memset((char *) PSDB, 1, window_size * sizeof(Packet));
  int i;
  for(i = 0; i < window_size; i++){
    PSDB[i].seq_num = i * MSS;
    PSDB[i].sent = 0;
    PSDB[i].acked = 0;
    PSDB[i].thread_on_duty = 0;
  }
  return;
}


// Extracts FIN from the packet
unsigned char extractFIN(Packet * pack){
  unsigned char fin = *((unsigned char *) &pack->header.fun_stuff[1]);
  return fin;
}

// Extracts seq_num from the number
int extractSeqNum(Packet * pack){
  unsigned int seq_num = *((int *)pack->header.seq_num);
  return seq_num; 
}

// Extracts ACK number
int extractACKNum(Packet * pack){
  unsigned int ack_num = *((int *)pack->header.ack_num);
  return ack_num;
}

// Extracts the checkSum
int extractCheckSum(Packet * pack){
  unsigned short checkSum = *((unsigned short *)pack->header.inet_checksum);
  return checkSum;
}

// Check correctness of the packet, write its data to the file as necessary
int processPacket(Packet * pack, char * filename){

  // Check the checkSum
  unsigned short checkSum = extractCheckSum(pack);
  memset(pack->header.inet_checksum, 0, 2);
  unsigned short newCheckSum = calculateChecksum(pack); 
  if(checkSum != newCheckSum){
    fprintf(stderr, "packet corrupted...\n");
    return 0;
  }

  int fin = extractFIN(pack);
  if(fin){
    return fin;
  }
  
  // Write data to the file
  int bytesReceived = pack->data_size;
  int offset = extractSeqNum(pack);

  // Prepare the structure for the writer thread 
  ToWriterThread * arg = (ToWriterThread *) malloc(sizeof(ToWriterThread));
  memset( (char *) arg, 0, sizeof(ToWriterThread));
  strcpy(arg->filename, filename);
  strcpy(arg->data, pack->data);
  arg->bytesReceived = bytesReceived;
  arg->offset = offset;

  // Creating the writer thread
  pthread_t writer;
  int err = pthread_create(&writer, NULL, writer_thread, (void *) arg);
  if(err != 0){
    fprintf(stderr, "threading failed\n");
    return -1;
  }
  
  return fin; 
}

// Create an ack from seq_num
Packet * createACK(int seq_num, unsigned short sport, unsigned short dport,
		   int fin){

  // Packet
  Packet * pack = (Packet * ) malloc(sizeof(Packet));
  memset((char *) pack, 0, sizeof(Packet));

  // Ports
  strncpy(pack->header.sport, (const char *) &sport, 2);
  strncpy(pack->header.dport, (const char *) &dport, 2);

  // Ack number as a seq number
  strncpy(pack->header.ack_num, (const char *) &seq_num, 4);

  // Set the FIN in case this packet signifies the end of transmission
  if(fin) pack->header.fun_stuff[1] |= 1 << 0;

  // Set data size to 0 since there is not data in ACK
  pack->data_size = 0;

  // Header length
  pack->header.fun_stuff[0] |= 1 << 4;
  pack->header.fun_stuff[0] |= 1 << 6;

  // Checksum
  unsigned short checkSum =  calculateChecksum(pack);
  strncpy(pack->header.inet_checksum, (const char *) &checkSum, 2); 

  return pack;
}

// Writer thread
void * writer_thread(void * arg){
  // Opening the file
  ToWriterThread * real_args = arg;
  FILE * fp = fopen(real_args->filename, "ab");
  fprintf(stderr, "writing into %s", real_args->filename);
  if(fp == NULL){
    die("failed to open the file:");
  }

  // Move to the offset position
  fseek(fp, real_args->offset, SEEK_SET);

  // Write the bytes
  size_t bytesWritten = 0;
  if( (bytesWritten = fwrite(real_args->data, 1, real_args->bytesReceived, fp))
      != real_args->bytesReceived){
    die("fwrite() failed: ");
  }

  // Cleanup
  fclose(fp);
  free(real_args);
  
  return NULL;
}








