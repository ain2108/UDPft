/*
 * author: Anton Nefedenkov 
 * email:  ain2108@columbia.edu
 * Program receives a file from the sender program. Secure file transmission over UDP.
 */


#include "packet.h"
#include "UDPsocket.h"
#include "logger.h"
#include "input.h"


int main(int argc, char ** argv){
  // Check correct number of args
  if(argc != 6){
    fprintf(stderr, 
	    "Usage: <%s> <filename> <listening_port> <sender_IP> <sender_port> "
	    "<log_filename>\n", argv[0]);
    return 0;
  }

  // Some declarations 
  char * file_name = argv[1];
  unsigned short listenPort = atoi(argv[2]);
  unsigned short senderPort = atoi(argv[4]);
  char * senderIP = argv[3];
  char * log_file = argv[5];

  FILE * fp = fopen(file_name, "wb");
  if(fp == NULL) die("fopen() failed:");
  

  // Before proceeding any further, check if the input is correct
  if(!isGoodInputSender(file_name, senderIP, listenPort, senderPort, 7)){
    return 0;
  }

  // This block necessary to get a valid IP from hostname. 
  char IP[SMALLBUFFER];
  memset(&IP, 0, SMALLBUFFER);
  int ipIsIPv4 = extractIP(senderIP, IP); // We now know if we should use v4 or v6

  // If ipv6, do not proceed any further.
  if(ipIsIPv4 != 1){
    fprintf(stdout, "IPv6 not yet supported. Exit.\n");
    return 0;
  }


  senderIP = IP;
  // Our sockets
  int sock = createIPv4UDPSocket();
  struct sockaddr_in * self = createIPv4Listener(listenPort, sock);
  int ack_sock = createIPv4UDPSocket();
  struct sockaddr_in * ackAddr = createIPv4ServAddr(senderPort, senderIP); 

  // Some more declarations
  Packet * ACK;
  Packet * pack;
  int seq_num;
  
  // Logger file
  FILE * log = fopen(log_file, "a");
  if(log == NULL){
    die("fopen() log failed:");
  }
  pthread_mutex_t log_lock;
  pthread_mutex_init(&log_lock, NULL);  
  
  char * myIP = getMyIP(ME_RECEIVER, IP_V_4);
  fprintf(stderr, "Waiting for delivery\n");

  int first = 0;
  int fin = 0;
  while(!fin){

    // Receive a packet
    pack = receivePacket(sock, self);
    
    // Notify the user that something is happening
    if(first == 0){
      fprintf(stdout, "Incoming transmission...\n");
      first = 1;
    }

    // Write data
    seq_num = extractSeqNum(pack);
    fin = processPacket(pack, fp);
    
    // Happens if packet was corrupted.
    if(fin == -1){
      fin = 0;
      continue;
    }
    
    // Log the status
    ToLoggerThread * logger_args = (ToLoggerThread *) malloc(sizeof(ToLoggerThread));
    memset((char *) logger_args, 0, sizeof(ToLoggerThread));
    logger_args->log = log;
    logger_args->log_lock = &log_lock;
    logger_args->sourceIP = senderIP;
    logger_args->destinationIP = myIP;
    logger_args->seq_num = seq_num;
    logger_args->ack_num = 0;
    logger_args->FIN = fin;
    logger_args->flag = ME_RECEIVER;
    logger_thread(logger_args);
  
    if(fin == 1) break;

    // Send ACK
    ACK = createACK(seq_num, listenPort, senderPort, fin);
    sendPacket(ack_sock, ackAddr, ACK);
    int extracted_ack_num = extractACKNum(ACK);
    free(ACK);

    // Log the status
    ToLoggerThread * logger_args2 = (ToLoggerThread *) malloc(sizeof(ToLoggerThread));
    memset((char *) logger_args2, 0, sizeof(ToLoggerThread));
    logger_args2->log = log;
    logger_args2->log_lock = &log_lock;
    logger_args2->sourceIP = myIP;
    logger_args2->destinationIP = senderIP;
    logger_args2->seq_num = 0;
    logger_args2->ack_num = extracted_ack_num;
    logger_args2->FIN = fin;
    logger_args2->flag = ME_RECEIVER;

    logger_thread(logger_args2);
    free(pack);
  }

  // Flood the sender with response FINS to let sender know that we are good
  int brute = 0;
  while(brute < 16){
    ACK = createACK(seq_num, listenPort, senderPort, fin);
    sendPacket(ack_sock, ackAddr, ACK);
    brute++;
  }

  // Cleanup
  sleep(2);
  fprintf(stdout, "Delivery completed successfully\n");
  free(self);
  free(ackAddr);
  fclose(log);
  fclose(fp);
  return 0;

}
