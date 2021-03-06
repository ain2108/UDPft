#include "packet.h"

// Function reads MSS bytes from offset, puts them into the buffer. 
int extractData(char * file_name, int offset, char * buffer){

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
    return bytesRead;
  }
  
  fclose(fp);
  return bytesRead;
}

// Copies an int into an Array
int intIntoCharArray(char * buffer, unsigned int number){

  char * temp = (char *)(&number);

  int i = 0;
  for(i = 0; i < sizeof(unsigned int); i++){
    buffer[i] = temp[i];
  }
  return 0;
}

// Copies a short into an array
int shortIntoCharArray(char * buffer, unsigned short number){

  char * temp = (char *)(&number);

  int i = 0;
  for(i = 0; i < sizeof(unsigned short); i++){
    buffer[i] = temp[i];
  }
  return 0;
}

// Function builds a complete packet, readu to be sent. 
Packet * buildPacket(char * file_name, unsigned short sport,
		unsigned short dport, unsigned int seq_num){

  // HEAP allocation
  Packet * pack = (Packet * ) malloc(sizeof(Packet));  
  memset((char *) pack, 0, sizeof(Packet));
  
  // Putting an int into the car array
  intIntoCharArray(pack->header.seq_num, seq_num);
  shortIntoCharArray(pack->header.sport, sport);
  shortIntoCharArray(pack->header.dport, dport);

  // Reading the data from file into the Packet
  int bytesRead = extractData(file_name, seq_num, pack->data);
  if(bytesRead == 0) return NULL; // In case fseek() after EOF

  // Set the header length. No options => 20 bytes long => 5 => 0101 0000
  pack->header.fun_stuff[0] |= 1 << 4;
  pack->header.fun_stuff[0] |= 1 << 6;

  // remmeber the number of bytes read as data
  pack->data_size = bytesRead;

  // Calculate the checksum and add it to the packet.
  unsigned short checkSum =  calculateChecksum(pack);
  shortIntoCharArray(pack->header.inet_checksum, checkSum);
  
  // printPacketHeader(pack);
  
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

// Function prints the details about the packet
void printPacketHeader(Packet * pack){

  unsigned short sport = *((unsigned short *)pack->header.sport);
  unsigned short dport = *((unsigned short *)pack->header.dport);
  unsigned int seq_num = *((int *)pack->header.seq_num);
  unsigned char fin = *((unsigned char *) &pack->header.fun_stuff[1]);
  unsigned char length = *((unsigned char *) &pack->header.fun_stuff[0]); 
  unsigned short checkSum = *((unsigned short *)pack->header.inet_checksum);
  unsigned int ack_num = *((int *)pack->header.ack_num);
  length = length >> 4;
  //  Printing it
  fprintf(stderr, "Printing packet header...\n");
  fprintf(stderr, "Source Port: %d\n", sport);
  fprintf(stderr, "Destination Port: %d\n", dport);
  fprintf(stderr, "<><><><><><><><><><><><><><><><><><>\n");
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
  fprintf(stderr, "<><><><><><><><><><><><><><><><><><>\n");
  return;
}

// Initiate the status DB
void initPacketStatusDB(PacketStatus * PSDB, int window_size){
  memset((char *) PSDB, 0, window_size * sizeof(PacketStatus));
  int i;
  for(i = 0; i < window_size; i++){
    PSDB[i].seq_num = i * MSS;
    PSDB[i].sent = 0;
    PSDB[i].available = 1;
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

// Function copies char array
int copyArrayToArray(char * buffer, char * source, int number){

  int i = 0;
  for(i = 0; i < number; i++){
    buffer[i] = source[i];
  }

  return 0;
}

// Check correctness of the packet, write its data to the file as necessary
int processPacket(Packet * pack, FILE * filename){

  // Check the checkSum
  unsigned short checkSum = extractCheckSum(pack);
  memset(pack->header.inet_checksum, 0, 2);
  unsigned short newCheckSum = calculateChecksum(pack); 
  
  // In case tha packet was corrupted
  if(checkSum != newCheckSum){
    return -1;
  }

  // Checking the FIN
  int fin = extractFIN(pack);
  if(fin){
    return fin;
  }
  
  // Write data to the file
  int bytesReceived = pack->data_size;
  if(bytesReceived == 0){
    fin = 1;
    return fin;
  }
  int offset = extractSeqNum(pack);

  // Prepare the structure for the writer thread 
  ToWriterThread * arg = (ToWriterThread *) malloc(sizeof(ToWriterThread));
  memset( (char *) arg, 0, sizeof(ToWriterThread));
  // strcpy(arg->filename, filename);
  arg->filename = filename;
  // strncpy(arg->data, pack->data, MSS);
  copyArrayToArray(arg->data, pack->data, bytesReceived);
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
  shortIntoCharArray(pack->header.sport, sport);
  shortIntoCharArray(pack->header.dport, dport);


  // Ack number as a seq number
  intIntoCharArray(pack->header.ack_num, seq_num);

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
static pthread_mutex_t file_lock = PTHREAD_MUTEX_INITIALIZER;
void * writer_thread(void * arg){
  // Opening the file
  ToWriterThread * real_args = arg;

  // Lock the file to prevent errors
  pthread_mutex_lock(&file_lock);
  // Move to the offset position
  FILE * fp = real_args->filename;
  fseek(fp, real_args->offset, SEEK_SET);
  // Write the bytes
  size_t bytesWritten = 0;
  if( (bytesWritten = fwrite(real_args->data, 1, real_args->bytesReceived, fp))
      != real_args->bytesReceived){
    die("fwrite() failed: ");
  }
  // Unlock the file
  pthread_mutex_unlock(&file_lock);

  // Cleanup
  free(real_args);
  return NULL;
}








