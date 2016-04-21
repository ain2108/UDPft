#include "helpers.h"
#include "input.h"
#include "packet.h"
#include "UDPsocket.h"
#include "controller.h"

int main(int argc, char ** argv){

  // Check correct number of args
  if(argc != 7){
    fprintf(stderr, 
	    "Usage: <%s> <filename> <remote_IP> <remote_port> <ack_port_num> "
	    "<log_filename> <window_size>\n", argv[0]);
    return 0;
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
    fprintf(stdout, "Send to IPv6 %s not supported. \n", IP);
  }

  return 0;
}



