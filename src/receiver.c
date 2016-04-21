#include "packet.h"
#include "UDPsocket.h"
#include "logger.h"

int main(int argc, char ** argv){


  // Some declarations 
  char * file_name = argv[1];
  unsigned short listenPort = atoi(argv[2]);
  unsigned short senderPort = atoi(argv[4]);
  char * senderIP = argv[3];
  char * log_file = argv[5];

  // Our sockets
  int sock = createIPv4UDPSocket();
  struct sockaddr_in * self = createIPv4Listener(listenPort, sock);
  int ack_sock = createIPv4UDPSocket();
  struct sockaddr_in * ackAddr = createIPv4ServAddr(senderPort, senderIP); 

  // Some more declarations
  Packet * ACK;
  Packet * pack;
  int seq_num;
 
  // Files
  FILE * fp = fopen(file_name, "wb");
  
  // Logger file
  FILE * log = fopen(log_file, "a");
  if(log == NULL){
    die("fopen() log failed:");
  }
  pthread_mutex_t log_lock;
  pthread_mutex_init(&log_lock, NULL);  
  
  // Will be set to (nil) if there was some problem recovering IP.
  char * myIP = getMyIP(ME_RECEIVER, IP_V_4);
  fprintf(stderr, "host ip: %s", myIP);


  int fin = 0;
  while(!fin){

    // Receive a packet
    pack = receivePacket(sock, self);
    printPacketHeader(pack);

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
    printPacketHeader(ACK);
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
    printPacketHeader(ACK);
    brute++;
  }
  // Cleanup
  sleep(2);
  fprintf(stdout, "file transfer complete.\n");
  free(self);
  free(ackAddr);
  fclose(log);
  fclose(fp);
  return 0;

}
