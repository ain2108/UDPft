#include "logger.h"
#include "controller.h"
#include "helpers.h"

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
    myIP[strlen(myIP) - 1] = '\0';
    return myIP;
  }else{
    getline(&myIP, &n, file);
    free(myIP);
    myIP = NULL;
    n = 0;
    getline(&myIP, &n, file);
    myIP[strlen(myIP) - 1] = '\0';
    return myIP;
  }
}

// Thread makes a log entry
void * logger_thread(void * arg){

  ToLoggerThread * real_args = (ToLoggerThread *) arg;
  
  // Buffer
  char entry[LOG_ENTRY_SIZE];
  memset(entry, 0, LOG_ENTRY_SIZE);

  time_t times = time(NULL);
  struct tm tm = *localtime(&times);

  // Get a time stampe
  sprintf(entry, "%d-%d-%d %d:%d:%d, ", 
	  tm.tm_year + 1900, 
	  tm.tm_mon + 1, 
	  tm.tm_mday, 
	  tm.tm_hour, 
	  tm.tm_min, 
	  tm.tm_sec);

  // Adding the sources and destinations
  strcat(entry, real_args->sourceIP);
  strcat(entry, ", ");
  strcat(entry, real_args->destinationIP);
  strcat(entry, ", ");
  
  // The ack numbers and seq numbers
  char integer[16];
  memset(integer, 0, 16);

  sprintf(integer, "%d, ", real_args->seq_num);
  strcat(entry, integer);
  memset(integer, 0, 16);
  
  sprintf(integer, "%d, ", real_args->ack_num);
  strcat(entry, integer);
  
  if(real_args->flag == 0){
    memset(integer, 0, 16);
    sprintf(integer, "%d sec ", TIME_OUT);
    strcat(entry, integer);
  }

  pthread_mutex_lock(real_args->log_lock);
  fprintf(real_args->log, "%s\n", entry);
  pthread_mutex_unlock(real_args->log_lock);

  free(real_args);
  return NULL;
}



