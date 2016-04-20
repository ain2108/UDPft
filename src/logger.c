#include "logger.h"


// If 0, get the ip adress of the sender for the sender, else receiver's IP for receiver
char * getMyIP(int receiver, int v6){
  
  FILE * file;
  if(receiver == 0){
    system(GET_SENDER_ADDRESS);
    file = fopen(SENDER_IP_FILE, "r");
  }else{
    system(GET_RECEIVER_ADDRESS);
    file = fopen(RECEIVER_IP_FILE, "r");
  }
  if(file == NULL) return NULL;

  // Not best function 2016
  char * myIP = NULL; //malloc(sizeof(char) * 46);
  size_t n = 0;
  // memset(myIP, 0, 46);

  if(v6 == 0){
    getline(&myIP, &n, file);  
    return myIP;
  }else{
    getline(&myIP, &n, file);
    free(myIP);
    myIP = NULL;
    n = 0;
    getline(&myIP, &n, file);
    return myIP;
  }
}
