#include "controller.h"

int initMuxedSocket(MuxedSocket * msock){
  msock->socket = createIPv4UDPSocket();
  pthread_mutex_init( &(msock->sock_lock), NULL);
  return 1;
}

int initSockMarket(MuxedSocket * market, int number){
  int i = 0;
  for(i = 0; i < number; i++){
    initMuxedSocket( &market[i]);
  }
  return 1;
}

int boss_threadIPv4(char * file_name, char * remote_IP,
		    unsigned short remote_port, unsigned short ack_port_num,
		    char * log_filename, int window_size){

  // Create mutliple sending sockets
  MuxedSocket * market = (MuxedSocket *) malloc(NUMBER_OF_ACTIVE_SOCKETS * sizeof(MuxedSocket));
  memset((char *) market, 0, NUMBER_OF_ACTIVE_SOCKETS * sizeof(MuxedSocket));
  struct sockaddr_in * receiverAddr = createIPv4ServAddr(remote_port, remote_IP);
  
  // Initiate all the sening sockets
  initSockMarket(market, NUMBER_OF_ACTIVE_SOCKETS);

  // Initiate the ack listener socket
  int ack_sock = createIPv4UDPSocket();
  struct sockaddr_in * self = createIPv4Listener(ack_port_num, ack_sock);

  // We need an array of size window size made of PacketStatus's
  PacketStatus * window = (PacketStatus *) malloc(window_size * sizeof(PacketStatus));
  memset((char *) window, 0, window_size * sizeof(PacketStatus));

  // Initiate the packet status array
  initPacketStatusDB(window, window_size);

  // We also need a read-write lock for the database
  pthread_rwlock_t window_lock;
  pthread_rwlock_init(&window_lock, NULL);

  // We also need a counter, that is going to be incremented when a slot is going to be
  // made unavailable because extractData read beyond EOF
  int transmission_complete = 0;
  pthread_mutex_t counter_lock;
  pthread_mutex_init(&counter_lock, NULL);  
  
  // Boss thread is going to walk the array in circle, asigning sending jobs to other threads 
  int i;
  for(i = 0; transmission_complete == window_size; i = (i + 1) % window_size){

    int seq_num;
    
    // Check if the slot is available
    pthread_rwlock_rdlock(&window_lock);
    if(window[i].available == 0 || window[i].sent == 1){
      pthread_rwlock_unlock(&window_lock);
      continue;
    }
    // Extract the sequence number and release the lock
    seq_num = window[i].seq_num;
    window[i].sent = 1; 
    pthread_rwlock_unlock(&window_lock);

    // Creating the args for sender_thread
    ToSenderThread * args = (ToSenderThread *) malloc(sizeof(ToSenderThread));
    args->slot = &window[i];
    args->window_lock = &window_lock;
    args->counter_lock = &counter_lock;
    args->market = market;
    args->receiverAddr = receiverAddr;
    args->seq_num = seq_num;
    args->position = i;
    args->file_name = file_name;
    args->sport = ack_port_num;
    args->dport = remote_port;
    args->counter = &transmission_complete;
    
    // Assign the sending of the packet to another thread
    pthread_t sender;
    int err = pthread_create(&sender, NULL, sender_thread, (void *) args);
    if(err != 0){
      die("threading at send failed: ");
    }
    pthread_rwlock_wrlock(&window_lock);
    window[i].thread_on_duty = sender;
    pthread_rwlock_unlock(&window_lock);

  }


  // Cleanup
  sleep(1);
  free(self);
  free(receiverAddr);
  free(window);
  // Close all sockets
  // Clean market
  // Close acking socket
  return 0;
}

// This thread sends the packet to the client
void * sender_thread(void * arg){

  ToSenderThread * real_args = arg;

  // Pick one of the sockets, be ready to lock on it.
  int dice = real_args->position % NUMBER_OF_ACTIVE_SOCKETS;
  MuxedSocket * mysocket = &(real_args->market[dice]);

  // Extract data creating a packet
  Packet * pack = buildPacket(real_args->file_name, real_args->sport,
			      real_args->dport,
			      real_args->seq_num);

  // Need to check. If pack == NULL, fseek() over EOF => make the slot unavailable.
  if(pack == NULL){

    // Signal that the slot is no longer available
    pthread_rwlock_wrlock(real_args->window_lock);
    real_args->slot->available = 0;
    pthread_rwlock_unlock(real_args->window_lock);

    // Increment the counter
    pthread_mutex_lock(real_args->counter_lock);
    ++(*(real_args->counter));
    pthread_mutex_unlock(real_args->counter_lock);

    free(real_args);
    return NULL;
  }

  // Send the packet
  pthread_mutex_lock(&(mysocket->sock_lock));
  sendPacket(mysocket->socket, real_args->receiverAddr, pack);
  pthread_mutex_unlock(&(mysocket->sock_lock)); 

  // Cleanup
  free(pack);
  free(real_args);
  return NULL;
}
