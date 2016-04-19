#ifndef _CONTROLLER_
#define _CONTROLLER_

#include "helpers.h"
#include "UDPsocket.h"
#include "packet.h"

#define NUMBER_OF_ACTIVE_SOCKETS 5

// Socket with locks
typedef struct MuxedSocket{
  int socket;
  pthread_mutex_t sock_lock;
  
} MuxedSocket;

// Many sockets with locks
typedef struct SockMarket{
  MuxedSocket msocks[NUMBER_OF_ACTIVE_SOCKETS];  
} SockMarket;

int initMuxedSocket();
int initSockMarket();




#endif
