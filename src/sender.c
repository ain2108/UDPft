#include "helpers.h"
#include "input.h"

int sendToIPv4(char * filename){

  return 1;
}

int sendToIPv6(char * filename){

  return 1;
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
    fprintf(stdout, "IPv4: %s\n", IP);  
  }else{
    fprintf(stdout, "IPv6: %s\n", IP);  
    
  }
  
  fprintf(stdout, "Good input:)\n");
  return 0;

}
