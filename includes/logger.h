#ifndef _LOGGER_
#define _LOGGER_

#include "helpers.h"

#define GET_SENDER_ADDRESS "./hostip.sh senderip.txt"
#define SENDER_IP_FILE "senderip.txt"

#define IP_V_6 1
#define IP_V_4 0
#define ME_SENDER 0
#define ME_RECEIVER 1

#define GET_RECEIVER_ADDRESS "./hostip.sh receiverip.txt"
#define RECEIVER_IP_FILE "receiverip.sh"

char * getMyIP(int receiver, int v6);

#endif
