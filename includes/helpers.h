#ifndef _HELPERS_
#define _HELPERS_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <pthread.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <netinet/in.h>

#define SMALLBUFFER 64

// Exit program 
void die(char * message);


#endif
