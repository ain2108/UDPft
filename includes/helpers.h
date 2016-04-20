#ifndef _HELPERS_
#define _HELPERS_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>


#include <pthread.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/ioctl.h>
#include <net/if.h>
#include <arpa/inet.h>


#define SMALLBUFFER 64
#define MAX_FILE_NAME 128

// Exit program 
void die(char * message);


#endif
