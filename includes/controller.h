#ifndef _CONTROLLER_
#define _CONTROLLER_

#include "helpers.h"
#include "UDPsocket.h"
#include "packet.h"

#define NUMBER_OF_ACTIVE_SOCKETS 5
#define TIME_OUT 1000000 // 1*10^6 us = 1 sec, this value is in microseconds!

// Socket with locks
typedef struct MuxedSocket{
  int socket;
  pthread_mutex_t sock_lock;
  
} MuxedSocket;

// Many sockets with locks
typedef struct SockMarket{
  MuxedSocket msocks[NUMBER_OF_ACTIVE_SOCKETS];  
} SockMarket;

typedef struct ToSenderThread{

  // Interesting part
  PacketStatus * slot;
  pthread_rwlock_t  * window_lock;
  int * counter;
  pthread_mutex_t * counter_lock;
  struct sockaddr_in * receiverAddr;
  int seq_num;
  int position;

  // Boring part
  char * file_name;
  unsigned short sport;
  unsigned short dport;
  
} ToSenderThread;

typedef struct ToAckerThread{

  PacketStatus * window;
  int window_size;
  unsigned short ack_port_num;
  pthread_rwlock_t  * window_lock;
  int * done;
  FILE * log;
  pthread_mutex_t * log_lock;
  char * myIP;
  char * remote_IP;
  
} ToAckerThread;

int initMuxedSocket(MuxedSocket * msock);
int initSockMarket(MuxedSocket * market, int number);
int boss_threadIPv4(char * file_name, char * remote_IP,
		    unsigned short remote_port, unsigned short ack_port_num,
		    char * log_filename, int window_size);
void * sender_thread(void * arg);
void * acker_thread(void * arg);
int findPosInWindow(int seq_num, PacketStatus * window, int window_size);


#endif
