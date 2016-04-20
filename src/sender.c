#include "helpers.h"
#include "input.h"
#include "packet.h"
#include "UDPsocket.h"
#include "controller.h"

int sendToIPv4(char * filename, char * remote_IP, int remote_port, int ack_port_num){

  return 1;
}

int sendToIPv6(char * filename){

  return 1;
}

void printBits(char * a, int numChars){

  int j = 0;
  while(j < numChars){
    int i;
    for (i = 7; i >= 0; i--) {
      printf("%d", !!((a[j] << i) & 0x80));
    }
    j++;
  }
  printf("\n");
  return;
}


int main(int argc, char ** argv){

  if(argc != 7){
    die("bad input\n");
  }

  // Extracting input
  char * file_name = argv[1];
  char * remote_IP = argv[2];
  int remote_port = atoi(argv[3]);
  int ack_port_num = atoi(argv[4]);
  char * log_filename = argv[5];
  int window_size = atoi(argv[6]);


  // Before proceeding any further, check if the input is correct
  if(!isGoodInputSender(file_name, remote_IP, remote_port, ack_port_num, window_size)){
    return 0;
  }

  // This block necessary to get a valid IP from hostname. 
  char IP[SMALLBUFFER];
  memset(&IP, 0, SMALLBUFFER);
  int ipIsIPv4 = extractIP(remote_IP, IP); // We now know if we should use v4 or v6

  if(0 == ipIsIPv4){
    die("Bad things\n");
  }else if(1 == ipIsIPv4){
    fprintf(stdout, "Sending to IPv4: %s\n", IP);
    boss_threadIPv4(file_name, IP, remote_port, ack_port_num, log_filename, window_size);
    
    
  }else{
    fprintf(stdout, "Sending to IPv6: %s\n", IP);
    
  }


  /*
  fprintf(stderr, "Sending %s...\n", argv[1]);
  Packet * pack = buildPacket(filename, ack_port_num, remote_port, 0);
  int sock = createIPv4UDPSocket();
  struct sockaddr_in * servAddr = createIPv4ServAddr(remote_port, IP);
  sendPacket(sock, servAddr, pack); 
  free(pack);
  */
  // fprintf(stdout, "size of short: %lu\n", sizeof(unsigned short));
  fprintf(stdout, "Good input:)\n");
  return 0;

}



