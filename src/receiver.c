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

  Packet * ACK;
  Packet * pack;
  int seq_num;

  // DBUGGING 
  FILE * fp = fopen(file_name, "wb");
  fseek(fp, 10000, SEEK_SET);
  char * end = "END";
  fwrite(end, 1, 4, fp);
  fclose(fp);
  
  int fin = 0;
  while(!fin){

    // Receive a packet
    pack = receivePacket(sock, self);
    printPacketHeader(pack);

    // Write data
    seq_num = extractSeqNum(pack);
    fin = processPacket(pack, file_name);
    free(pack);

    // Send ACK
    ACK = createACK(seq_num, listenPort, senderPort, fin);
    sendPacket(ack_sock, ackAddr, ACK);
    //    printPacketHeader(ACK);
    free(ACK);
    sleep(1);
    
  }

  // Flood the sender with response ACKs
  int brute = 0;
  while(brute < 5){
    ACK = createACK(seq_num, listenPort, senderPort, fin);
    sendPacket(ack_sock, ackAddr, ACK);
  }
  // Cleanup
  sleep(1);
  fprintf(stdout, "file transfer complete.\n");
  free(self);
  free(ackAddr);
  return 0;

}
