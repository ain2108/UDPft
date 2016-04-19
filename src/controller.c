#include "controller.h"


int initMuxedSocket(MuxedSocket * msock){
  msock->socket = createIPv4UDPSocket();
  pthread_mutex_init( &(msock->sock_lock), NULL);
  return 1;
}

int initSockMarket(MuxedSocket * market, int number){
  int i = 0;
  for(i = 0; i < number; i++){
    initMuxedSocket( &market[i]);
  }
  return 1;
}
