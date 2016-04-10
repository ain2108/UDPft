#ifndef _INPUT_
#define _INPUT_

int isGoodInputSender(char * filename, char * remote_IP, int remote_port,
		      int ack_port_num, int window_size);
int isGoodPort(int port);
int isValidAdress(char * IP);
int isValidHostname(char * hostname);
int extractIP(char * IP, char * extractedIP);
int ipFromHostname(char * hostname, char * extractedIP);


#endif
