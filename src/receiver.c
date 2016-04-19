#include "packet.h"
#include "UDPsocket.h"

int main(int argc, char ** argv){


  // Some declarations 
  char * file_name = argv[1];
  unsigned short listenPort = atoi(argv[2]);
  unsigned short senderPort = atoi(argv[4]);
  char * senderIP = argv[3];
  char * log_file = argv[5];
  
  int sock = createIPv4UDPSocket();
  struct sockaddr_in * self = createIPv4Listener(listenPort, sock);
  int ack_sock = createIPv4UDPSocket();
  struct sockaddr_in * ackAddr = createIPv4ServAddr(senderPort, senderIP); 
  
  
  int fin = 0;
  while(!fin){

    // Receive a packet
    Packet * pack = receivePacket(sock, self);
    printPacketHeader(pack);

    // Write data
    int seq_num = extractSeqNum(pack);
    int fin = processPacket(pack, file_name);
    free(pack);

    // Send ACK
    Packet * ACK = createACK(seq_num, listenPort, senderPort, fin);
    sendPacket(ack_sock, ackAddr, ACK);
    printPacketHeader(ACK);
    free(ACK);
    
  }

  // Cleanup
  sleep(1);
  fprintf(stdout, "file transfer complete.\n");
  free(self);
  free(ackAddr);
  return 0;

}
