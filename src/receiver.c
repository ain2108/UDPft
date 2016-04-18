#include "packet.h"
#include "UDPsocket.h"


int main(int argc, char ** argv){


  char * file_name = argv[1];
  unsigned short listenPort = atoi(argv[2]);
  unsigned short senderPort = atoi(argv[4]);
  char * senderIP = argv[3];
  char * log_file = argv[5];

  
  int sock = createIPv4UDPSocket();
  struct sockaddr_in * self = createIPv4Listener(listenPort, sock); 

  Packet * pack = receivePacket(sock, self);
  printPacketHeader(pack);

  free(pack);
  free(self);
  return 0;

}
