#include "input.h"
#include "helpers.h"

// Function checks if the input is correct
int isGoodInputSender(char* filename, char * remote_IP, int remote_port,
		      int ack_port_num, int window_size){

  // Check that the file exists and can be opened
  FILE * file = fopen(filename, "rb");
  if(file == NULL){
    die("Error while opening the file");
    return 0;
  }
  
  // Check the port numbers
  if(!isGoodPort(ack_port_num)){
    die("Bad ACK port number. Exit.\n");
    return 0;
  }
  if(!isGoodPort(remote_port)){
    die("Bad remote port number. Exit.\n");
    return 0;
  }

  // Check for normality of windowsize.
  if(window_size <= 0){
    fprintf(stderr,"Bad windowsize. Using default: %s\n", DEFAULT_WINDOW_SIZE);
    return 0;
  }

  // Check validity of whatever adress was provided
  if(!isValidAdress(remote_IP)){
    die("Bad address for remote server");
    return 0;
  }
  return 1;
}

// Checks the validity of port 
int isGoodPort(int port){
  if(1024 <= port && port <= 65535){
    return 1;
  }
  return 0;
}

// Checks if the provided andress is valid
int isValidAdress(char * IP){

  struct sockaddr_in saIPv4;
  if(inet_pton(AF_INET, IP, &(saIPv4.sin_addr))){
    return 1;
  }

  struct sockaddr_in6 saIPv6;
  if(inet_pton(AF_INET6, IP, &(saIPv6.sin6_addr))){
    return 1;
  }

  if(isValidHostname(IP)){
    return 1;
  }
  return 0;
}

// Function checks if the hostname is valid
int isValidHostname(char * hostname){

  struct addrinfo hints, *servinfo;
  int rv;

  memset(&hints, 0, sizeof(hints));
  hints.ai_family = AF_UNSPEC; 
  hints.ai_socktype = SOCK_STREAM;

  // If hostname is invalid,
  if ((rv = getaddrinfo(hostname, "http", &hints, &servinfo)) != 0) {
    return 0;
  }

  freeaddrinfo(servinfo);
  return 1;
}

/* Function writes checked IP adress into extractedIP buffer
   Returns 0 on failure, 1 if extracted IP is IPv4, 2 if extracted ip is IPv6 */  
int extractIP(char * IP, char * extractedIP){

  // If IP is already in IPv4, we are done
  struct sockaddr_in saIPv4;
  if(inet_pton(AF_INET, IP, &(saIPv4.sin_addr))){
    strcpy(extractedIP, IP);
    return 1;
  }

  // If IP is already in IPv6, we are done
  struct sockaddr_in6 saIPv6;
  if(inet_pton(AF_INET6, IP, &(saIPv6.sin6_addr))){
    strcpy(extractedIP, IP);  
    return 2;
  }

  // If IP is not IP but a hostname
  if(isValidHostname(IP)){
    return ipFromHostname(IP, extractedIP);    
  }
  return 0;
  
}

/* Function extracts an IP adress from the hostname.
   Returns 0 on failure, 1 if extracted IP is IPv4, 2 if extracted ip is IPv6 */
int ipFromHostname(char * hostname, char * extractedIP){

  struct addrinfo hints, *servinfo;
  int rv;

  memset(&hints, 0, sizeof(hints));
  hints.ai_family = AF_UNSPEC; 
  hints.ai_socktype = SOCK_STREAM;

  // Get the information
  if ((rv = getaddrinfo(hostname, "http", &hints, &servinfo)) != 0) {
    return 0;
  }

  // Figure out which family it relates to. 
  if(servinfo->ai_family == AF_INET){
    // Will extract IPv4
    inet_ntop(AF_INET, &(((struct sockaddr_in *)(servinfo->ai_addr))->sin_addr),
	      extractedIP, INET_ADDRSTRLEN);
    freeaddrinfo(servinfo);
    return 1;
    
  }else if(servinfo->ai_family == AF_INET6){
    // Will extract IPv6
    inet_ntop(AF_INET6, &(((struct sockaddr_in6 *)(servinfo->ai_addr))->sin6_addr),
	      extractedIP, INET6_ADDRSTRLEN);
    freeaddrinfo(servinfo);
    return 2;
  
  }else{
    die("Some ungodly stuff is happening\n");
    return 0;
  }
  return 0;
}
