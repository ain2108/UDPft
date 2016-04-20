#include "UDPsocket.h"

// Creates IPv4 UDP socket
int createIPv4UDPSocket(){
  int sock;
  if ( (sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) == -1){
    die("socket() failed:");
  }
  return sock;
}

// Function fills in the server address structure for use with a UDP socket.
struct sockaddr_in * createIPv4ServAddr(unsigned short port, char * IPv4){

  struct sockaddr_in * servAddr = (struct sockaddr_in *) malloc(sizeof(struct sockaddr_in));
 
  memset((char *) servAddr, 0, sizeof(struct sockaddr_in));
  servAddr->sin_family = AF_INET;
  servAddr->sin_port = htons(port);
  if(inet_aton(IPv4, &servAddr->sin_addr) == 0){
    die("inet_aton() failed:");
  }

  return servAddr;
}

// Function fills in the serv address on the receiver side
struct sockaddr_in * createIPv4Listener(unsigned short port, int sock){
  
  struct sockaddr_in * servAddr = (struct sockaddr_in *) malloc(sizeof(struct sockaddr_in));
  int servLen = sizeof(struct sockaddr_in);

  // Filling the structure
  memset((char *) servAddr, 0, servLen);
  servAddr->sin_family = AF_INET;
  servAddr->sin_port = htons(port);
  servAddr->sin_addr.s_addr = htonl(INADDR_ANY);
  if(bind(sock, (struct sockaddr *)servAddr, sizeof(struct sockaddr_in)) == -1){
    die("bind() failed:");
  }
  
  return servAddr;
}

// Send a UDP packet through the socket.
int sendPacket(int sock, struct sockaddr_in * servAddr, Packet * pack){

  // printf("Sending packet...\n");
 
  // Send the packet
  if(sendto(sock, (char *) pack, pack->data_size + 20, 0,
	    (struct sockaddr *)servAddr, sizeof(struct sockaddr_in)) == -1){
    die("sendto() failed:");
  }

  return 1;
}

// Receives a packet
Packet * receivePacket(int sock, struct sockaddr_in * servAddr){
  // Creating a packet
  Packet * pack = (Packet *) malloc (sizeof(Packet));
  memset((char *) pack, 0, sizeof(Packet));
  socklen_t servLen = sizeof(struct sockaddr_in);

  ssize_t bytesReceived;
  if((bytesReceived = recvfrom(sock, (char *) pack, MSS + 20, 0,
			       (struct sockaddr *)servAddr, &servLen)) ==-1){
    die("recvfrom() failed:");
  }
  int dataLength = bytesReceived - 20;
  pack->data_size = dataLength;
  
  return pack;
}

// Get my IPv4 address on eth0
// broken
char * getMyIPv4(){

  int sock;
  struct ifreq addresses;
  sock = socket(AF_INET, SOCK_DGRAM, 0);
  strncpy(addresses.ifr_name, "eth0", IFNAMSIZ-1); // Which interface
  addresses.ifr_addr.sa_family = AF_INET; // Which family
  ioctl(sock, SIOCGIFNETMASK, &addresses);

  char * MyIPv4 = (char *) malloc(16);
  memset(MyIPv4, 0, 16);

  strncpy(MyIPv4, inet_ntoa(((struct sockaddr_in *) &addresses.ifr_addr)->sin_addr), 16);
  return MyIPv4;
}





