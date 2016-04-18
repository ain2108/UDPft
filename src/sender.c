#include "helpers.h"
#include "input.h"
#include "boss_thread.h"

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

  // Extracting input
  char * filename = argv[1];
  char * remote_IP = argv[2];
  int remote_port = atoi(argv[3]);
  int ack_port_num = atoi(argv[4]);
  char * log_filename = argv[5];
  int window_size = atoi(argv[6]);


  // Before proceeding any further, check if the input is correct
  if(!isGoodInputSender(filename, remote_IP, remote_port, ack_port_num, window_size)){
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
    
  }else{
    fprintf(stdout, "Sending to IPv6: %s\n", IP);
    
  }

  /*
  unsigned short test = 1;
  fprintf(stdout, "test: %d\n", test);
  char a[2];
  strncpy(a, (const char *) &test, 2);
  printBits(a, 2);
  test = *((int *)a);
  fprintf(stderr, "test dec: %d\n", test);
  */
  

  fprintf(stderr, "Sending %s...\n", argv[1]);
  Packet * pack = buildPacket(filename, ack_port_num, remote_port, 0);
  free(pack);
  // fprintf(stdout, "size of short: %lu\n", sizeof(unsigned short));
  fprintf(stdout, "Good input:)\n");
  return 0;

}



