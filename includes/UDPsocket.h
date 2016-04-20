#ifndef _UDP_SOCKET_
#define _UDP_SOCKET_

#include "helpers.h"
#include "packet.h"


// IPv4
int createIPv4UDPSocket();
struct sockaddr_in * createIPv4ServAddr(unsigned short port, char * IPv4);
struct sockaddr_in * createIPv4Listener(unsigned short port, int sock);
int sendPacket(int sock, struct sockaddr_in * servAddr, Packet * pack);
Packet * receivePacket(int sock, struct sockaddr_in * servAddr);
char * getMyIPv4();


#endif
