//
// Copyright 2015 Chaudhry Junaid Anwar
//



// link layer file

#include <string>
#include <queue>

#define DEBUG_THIS_FILE
#include "debug.hh"
#include "externals.hh"
#include "error.hh"
#include "threads.hh"
#include "udp.hh"
#include "fileparser.hh"
#include "hdlcframe.hh"

#define MAX_MTU 16384


Udpsocket *comm_socket;
Udpsocket *recv_socket;
pthread_mutex_t comm_socket_mutex;

queue<dgram> link_send_queue;
queue<dgram> link_recv_queue;
pthread_mutex_t link_send_queue_mutex;
pthread_mutex_t link_recv_queue_mutex;

sem_t link_send_queue_has_data;
sem_t link_recv_queue_has_data;


void init_link_layer(int host_node_id)
{
  comm_socket = new Udpsocket[ext::total_nodes];
  recv_socket = new Udpsocket;

  // must be greater than 0
  assert(ext::total_nodes);

  for (int i=0; i<ext::total_nodes;i++){
    if (i==host_node_id){
      recv_socket->bind_socket("localhost.localdomain",50000+i);
      comm_socket[i].fix_dest("localhost.localdomain",50000+i);
    }else{
      comm_socket[i].fix_dest("localhost.localdomain",50000+i);
    }
  }

  dbglog("Quantizing channel flakiness... ");
  dbglog("Approximately 1 packet in " << (int)ext::packet_drop_wt << " will have a 1-bit error.");

  dbglog("Quantizing network mischief...");
  dbglog("Approximately 1 packet in " << (int)ext::packet_err_wt << " will be dropped."); 

  checkval_0(sem_init(&link_send_queue_has_data,0,0),Exception("init_link_layer","semaphore could not be initialized."));
  checkval_0(sem_init(&link_send_queue_has_data,0,0),Exception("init_link_layer","semaphore could not be initialized."));
  checkval_0(pthread_mutex_init( &comm_socket_mutex,NULL),
	     Exception("init_link_layer","mutex could not be initialized.")
	     );
  checkval_0(pthread_mutex_init( &link_send_queue_mutex,NULL),
	     Exception("init_link_layer","mutex could not be initialized.")
	     );
  checkval_0(pthread_mutex_init( &link_recv_queue_mutex,NULL),
	     Exception("init_link_layer","mutex could not be initialized.")
	     );
}

void* link_send_element(void* arg) 
{
  fin("link_send_element");
  while (!ext::time_to_quit){
    sem_wait(&link_send_queue_has_data);
    dbglog("link_send resumed...");
    pthread_mutex_lock(&link_send_queue_mutex);
    while (!link_send_queue.empty()){
      dgram a_dgram(link_send_queue.front());
      link_send_queue.pop();
      pthread_mutex_unlock(&link_send_queue_mutex);

      int address = a_dgram.address;
      if (address>=ext::total_nodes){
	dbglog("Packets has unrecognized forwarding address, dropping it now!");
	pthread_mutex_lock(&link_send_queue_mutex);
	continue;
      }
      // push my address at back to enable prev_hop calculation at the receiver
      a_dgram.frame.push_back(ext::my_address);
      pthread_mutex_lock(&comm_socket_mutex);
      dbglog("(*Sending frame*) "  << a_dgram.frame);
      comm_socket[address].send_dgram(a_dgram.frame.data(),a_dgram.frame.length());
      pthread_mutex_unlock(&comm_socket_mutex);
      pthread_mutex_lock(&link_send_queue_mutex);
    }
    pthread_mutex_unlock(&link_send_queue_mutex);
  }
  pthread_exit(0);
}

// deprecated function
char* get_buffer(string s)
{
  char *buffer = new char[s.size()];
  for (unsigned i=0;i<s.size();i++){
    buffer[i] = s[i];
  }

  return buffer;
}

void* link_recv_element(void* arg)
{
  fin("link_recv_element");
  
  dgram adgram;
  char str[MAX_MTU] = {0};
  int intr_err = 0;
  while (!ext::time_to_quit){
    recv_socket->recv_dgram(str,MAX_MTU);
    
    if ( ext::packet_drop_wt!=0 && (rand()%ext::packet_drop_wt == 0) ){
      dbglog( "Ha Ha Ha! Dropped a packet.");
      continue;
    }else if ( ext::packet_err_wt!=0 &&  (rand()%ext::packet_err_wt==0) ){
      intr_err = rand() % ext::max_drop_idx;
      dbglog( "Error in coming stream at (random) location:  " << intr_err);
    }else{
      dbglog("No error! Am I talking over a real channel or what?");
      intr_err = 0; // no errors
    }

    bool in_frame = false;
    adgram.frame.clear();
    for (unsigned long i=0;i<MAX_MTU;i++){
      if (str[i]==flag){
	if (in_frame){
	  adgram.frame.push_back(str[i]);
	  adgram.address = str[i+1];
	  break; // terminating flag reached, end loop here
	}
	in_frame = true;
      }
      if (intr_err>0 && i==(unsigned)intr_err){
	str[i]^=0x01; // invert last bit
	dbglog( "Introduced a 1-bit error.");
      }
      adgram.frame.push_back(str[i]);
    }
    dbglog("LINK: [PacketReceived]: " << adgram.frame << " from " << (int)adgram.address) ;

    pthread_mutex_lock(&link_recv_queue_mutex );
    link_recv_queue.push(adgram);
    pthread_mutex_unlock(&link_recv_queue_mutex);
    sem_post(&link_recv_queue_has_data);
  }
  fout("link_recv_element");
  pthread_exit(0);
}

void* link_layer(void* arg) 
{
  fin("link_layer()");
  int host_node_id = (long)arg;
  pthread_t tid;
  
  // initialize layer
  init_link_layer(host_node_id);
  
  // spawn receive element
  int rv = pthread_create(&tid,NULL,link_recv_element,NULL);
  checkval_0(rv,Exception("link_layer()","could not spawn receive element"));
  pthread_detach(tid);
  
  // spawn sending element
  rv = pthread_create(&tid,NULL,link_send_element,NULL);
  checkval_0(rv,Exception("link_layer()","could not spawn sending element"));
  pthread_detach(tid);

  fout("link_layer()");
  pthread_exit(0);
}

