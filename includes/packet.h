#ifndef _PACKET_
#define _PACKET_

#include "helpers.h"

#define MSS 512

typedef struct PacketStatus{
  int seq_num;
  int sent;
  int available;
  pthread_t thread_on_duty;
  
} PacketStatus;

typedef struct TCPHeader{

  char sport[2];
  char dport[2];
  char seq_num[4];
  char ack_num[4];
  char fun_stuff[2];
  char rec_window[2];
  char inet_checksum[2];
  char more_fun_stuff[2];
    
} TCPHeader;

typedef struct Packet{
  TCPHeader header;
  char data[MSS];
  int data_size;
} Packet;

typedef struct ToWriterThread{
  FILE * filename;
  char data[MSS];
  int bytesReceived;
  int offset;  
} ToWriterThread;

void initPacketStatusDB(PacketStatus * PSDB, int size);
int extractData(char * file_name, int offset, char * buffer);
Packet * buildPacket(char * file_name, unsigned short sport,
		unsigned short dport, unsigned int seq_num);
void printPacketHeader(Packet * pack);
unsigned short calculateChecksum(Packet * pack);
void * writer_thread(void * arg);
int processPacket(Packet * pack, FILE * filename);
unsigned char extractFIN(Packet * pack);
int extractSeqNum(Packet * pack);
int extractACKNum(Packet * pack);
int extractCheckSum(Packet * pack);

Packet * createACK(int seq_num, unsigned short sport, unsigned short dport, int fin);
void * writer_thread(void * arg); 

#endif
