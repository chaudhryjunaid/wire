//
// Copyright 2015 Chaudhry Junaid Anwar
//


// datalink layer

#define DEBUG_THIS_FILE
#include "debug.hh"

#include <queue>
#include <vector>
#include <queue>
#include <errno.h>
#include "threads.hh"
#include "datalink.hh"
#include "hdlcframe.hh"
#include <sys/time.h>

using namespace std;

typedef packet Packet_t;


queue<Packet_t> dlink_send_queue;
queue<Packet_t> dlink_recv_queue;

pthread_mutex_t dlink_send_queue_mutex;
pthread_mutex_t dlink_recv_queue_mutex;

sem_t dlink_send_queue_has_data;
sem_t dlink_recv_queue_has_data;

typedef vector<packet> Packet_array_t;
typedef char Seq_num_t;


struct Ack
{
  char address;			// address to which to send
  Seq_num_t seq_num;		// seq num of packet to ack
  Ack(char addr,Seq_num_t seq):address(addr),seq_num(seq)
  {}
};

queue<Ack> ack_send_queue;	// acknowledgements to send to the senders
queue<Ack> ack_recv_queue;	// acks received from receivers

pthread_mutex_t ack_send_queue_mutex;
pthread_mutex_t ack_recv_queue_mutex;

sem_t ack_send_queue_has_data;
sem_t ack_recv_queue_has_data;

// deprecated
// struct Send_packet
// {
//   Packet_t p;			// actual packet
//   Seq_num_t seq_num;		// 7-bit sequence number
//   time_t send_time;		// field to store time the packet was sent
//   Send_packet(packet pack,int seqnum):p(pack),seq_num(seqnum),send_time(0)
//   {}
//   Send_packet(){}
// };


struct Send_packet
{
  Packet_t p;			// actual packet
  Seq_num_t seq_num;		// 7-bit sequence number
  timeval send_time;		// struct to store time the packet was sent
  Send_packet(packet pack,int seqnum):p(pack),seq_num(seqnum)
  {}
  Send_packet(){}
};

struct Send_window_state_t
{
  Seq_num_t LFS;		// last frame sent
  Send_packet sent_packet;
};

struct Recv_window_state_t
{
  Seq_num_t NFE;		// next frame expected
  Send_packet recvd_packet;
};

// deprecated
// inline bool timeout(Send_packet sp,time_t to_val)
// {
//   return (time(NULL)-sp.send_time)>to_val;
// }

// deprecated
// inline bool timeout(Send_packet sp)
// {
//   return (clock()-sp.send_time) > ext::to_val;
// }


/* Subtract the `struct timeval' values X and Y,
   storing the result in RESULT.
   Return 1 if the difference is negative, otherwise 0.  */

// reference: GNU LIBC MANUAL (chapter # 21; section 2)

int timeval_subtract (timeval *result,timeval *x,timeval *y)
{
  /* Perform the carry for the later subtraction by updating Y. */
  if (x->tv_usec < y->tv_usec) {
    int nsec = (y->tv_usec - x->tv_usec) / 1000000 + 1;
    y->tv_usec -= 1000000 * nsec;
    y->tv_sec += nsec;
  }
  if (x->tv_usec - y->tv_usec > 1000000) {
    int nsec = (x->tv_usec - y->tv_usec) / 1000000;
    y->tv_usec += 1000000 * nsec;
    y->tv_sec -= nsec;
  }

  /* Compute the time remaining to wait.
     `tv_usec' is certainly positive. */
  result->tv_sec = x->tv_sec - y->tv_sec;
  result->tv_usec = x->tv_usec - y->tv_usec;

  /* Return 1 if result is negative. */
  return x->tv_sec < y->tv_sec;
}

inline bool timeout(Send_packet sp)
{
  timeval curr_time;
  gettimeofday(&curr_time,0);

  timeval time_since_send;
  timeval_subtract(&time_since_send,&curr_time,&sp.send_time);

  return (timeval_subtract(&curr_time,&ext::to_val,&time_since_send) == 1 );
}

void init_dlink_layer(void)
{
  fin("init_dlink_layer()");
  checkval_0(sem_init(&dlink_send_queue_has_data,0,0),
	     Exception("init_dlink_layer","semaphore could not be initialized."));
  checkval_0(sem_init(&dlink_recv_queue_has_data,0,0),
	     Exception("init_dlink_layer","semaphore could not be initialized."));
  checkval_0(pthread_mutex_init( &dlink_send_queue_mutex,NULL),
	     Exception("init_dlink_layer","mutex could not be initialized.") );
  checkval_0(pthread_mutex_init( &dlink_recv_queue_mutex,NULL),
	     Exception("init_dlink_layer","mutex could not be initialized.") );

  checkval_0(sem_init(&ack_send_queue_has_data,0,0),
	     Exception("init_dlink_layer","semaphore could not be initialized."));
  checkval_0(sem_init(&ack_recv_queue_has_data,0,0),
	     Exception("init_dlink_layer","semaphore could not be initialized."));
  checkval_0(pthread_mutex_init( &ack_send_queue_mutex,NULL),
	     Exception("init_dlink_layer","mutex could not be initialized.") );
  checkval_0(pthread_mutex_init( &ack_recv_queue_mutex,NULL),
	     Exception("init_dlink_layer","mutex could not be initialized.") );
  fout("init_dlink_layer()");
}

void flush_acks()
{
  fin("flush_acks()");

  pthread_mutex_lock(&ack_send_queue_mutex);

  // should never be empty because of calling sequence
  if (ack_send_queue.empty()){
    dbglog("*** The send queue is empty.");
  }

  while ( !ack_send_queue.empty() ) {
    dbglog("Current length (ack_send_queue): " << (int)ack_send_queue.size() );

    Ack send_ack(ack_send_queue.front());
    ack_send_queue.pop(); 
     
    packet framed_packet;
    framed_packet.frame = "[#]"; // cannot allow zero byte payload currently
    framed_packet.address = send_ack.address;
    short control= mk_sctrl(0,0,send_ack.seq_num);
    framed_packet.frame = frame(framed_packet.address,control,framed_packet.frame.data(),framed_packet.frame.size() );
      
    dbglog("flush_acks(): Sending ack: " << framed_packet.frame << "  to  " << (int)framed_packet.address );
      
    pthread_mutex_lock(&link_send_queue_mutex);
    link_send_queue.push( framed_packet );
    pthread_mutex_unlock(&link_send_queue_mutex);
    sem_post(&link_send_queue_has_data);
  }
  pthread_mutex_unlock(&ack_send_queue_mutex);

  fout("flush_acks()");
}

void* dlink_send_element(void* arg) 
{
  fin("datalink_send_element()");

  Packet_array_t link[ext::total_nodes];
  Send_window_state_t send_window[ext::total_nodes];
  int timeouts = 0;
  bool timed_out = false;
  bool ack_arrived = false;

  // initialize send windows' states
  for (int i=0;i<ext::total_nodes;i++){ // run all sliding window protocols sequentially  
    send_window[i].LFS = 1;	//start swp from 0
  }
  
  while (!ext::time_to_quit){
    
    // assuming that sem_wait never encounters any error
    sem_wait(&dlink_send_queue_has_data);
    dbglog("Datalink_send_data resumed...");
      
    // grab all packets from the datalink send queue
    // and store in vector for appropriate link
    pthread_mutex_lock(&dlink_send_queue_mutex);
    while (!dlink_send_queue.empty()){
      Packet_t a_pack(dlink_send_queue.front());
      dlink_send_queue.pop();
      link[(int)a_pack.address].push_back(a_pack);
    }
    pthread_mutex_unlock(&dlink_send_queue_mutex);

    // run ARQ for all destinations sequentially
    for (int i=0;i<ext::total_nodes;i++) {

      while (!link[i].empty()) {
	// avoid confusion from stray acknowledgements
// 	pthread_mutex_lock(&ack_recv_queue_mutex);
// 	while (!ack_recv_queue.empty())
// 	  ack_recv_queue.pop();
// 	pthread_mutex_unlock(&ack_recv_queue_mutex);

	// invert to get next packet sequence number
	if (send_window[i].LFS){
	  send_window[i].LFS = 0;
	}else {
	  send_window[i].LFS = 1; 
	}

	// frame next packet from queue front
	packet framed_packet = link[i].front();
	short control= mk_ictrl(0,0,send_window[i].LFS);
	framed_packet.frame = frame(framed_packet.address,control,framed_packet.frame.data(),framed_packet.frame.size() );
	
	// fetch packet (dequeue from original) and store as current
	send_window[i].sent_packet = Send_packet(framed_packet, send_window[i].LFS);
	link[i].erase(link[i].begin());

	// init state
	ack_arrived = false;
	timed_out = false;
	timeouts = 0;
	do{
	  pthread_mutex_lock(&link_send_queue_mutex);
	  gettimeofday(&(send_window[i].sent_packet.send_time),0 );
	  link_send_queue.push( send_window[i].sent_packet.p );
	  pthread_mutex_unlock(&link_send_queue_mutex);
	  sem_post(&link_send_queue_has_data);
	  
	  dbglog("Packet sent.");
	  dbglog("Waiting for Ack to arrive...");
	  while ( 1 ){
	    if (  sem_trywait(&ack_recv_queue_has_data) == 0){
	      pthread_mutex_lock(&ack_recv_queue_mutex);
	      Ack a = ack_recv_queue.front();
	      ack_recv_queue.pop();
	      pthread_mutex_unlock(&ack_recv_queue_mutex);
	      dbglog("Seq_num of ack received =  " << (int)a.seq_num);

	      if (a.seq_num!=send_window[i].LFS){
		dbglog( "Ack received but sequence number does not match.");
		continue;
	      }else{
		dbglog( "Ack sequence number matched.");
		ack_arrived = true;
		break;
	      }
	    }else if ( timeout(send_window[i].sent_packet) ){
	      
	      timed_out = true;
	      break;
	    }
	  } // while ack arrives, wait

	  if ( ack_arrived ) {
 	    dbglog( "Acknowlegement accepted.");
 	    break;		// break do while
 	  } else if ( timed_out ) {
	    timeouts++; timed_out = false;
	    dbglog( "Timeout occured " << timeouts << " time.");
	  }
	}while( timeouts<ext::max_timeouts && !ack_arrived) ;
	
	if(timeouts>=ext::max_timeouts){
	  dbglog( "Destination node unreachable.");
	  break; //maybe notify network layer in future versions
	} // 
	
      }//while link [i]
    }//  seq arq
    
  } // ! time to quit
  
  fout("datalink_send_element()");
  pthread_exit(0);
}

void* dlink_recv_element(void* arg)
{
  fin("datalink_recv_element()");
  
  Recv_window_state_t recv_window[ext::total_nodes];
  char sender_address;
  short control;
  Seq_num_t dummy_seq; //piggy backing not used
  bool dummy_PF_bit;
  Seq_num_t seq_num;

  // init recv window states
  for (int i=0;i<ext::total_nodes;i++){  
    recv_window[i].NFE = 0; //start swp from 0
  }

  while (!ext::time_to_quit){

    sem_wait(&link_recv_queue_has_data);
   
    // fetch packet from network
    pthread_mutex_lock(&link_recv_queue_mutex);
    while(!link_recv_queue.empty()){

      packet p( link_recv_queue.front() );
      link_recv_queue.pop();
      pthread_mutex_unlock(&link_recv_queue_mutex);
      
      sender_address = p.address;
      dbglog("Received a frame from " << (int)sender_address);
      if ( deframe(p.address,control,p.frame) ) {
	dbglog("Cyclic Redundency Check error: dropping packet.");  
	pthread_mutex_lock(&link_recv_queue_mutex);
	continue; 		// go for next frame
      }

      if (is_ictrl(control)) {
	dbglog("Information frame received from " << (int)sender_address);
	// if information frame, send to above layer
	bk_ictrl(dummy_seq,dummy_PF_bit,seq_num,control);
	if (  seq_num == recv_window[(int)sender_address].NFE  ){
	  pthread_mutex_lock(&dlink_recv_queue_mutex);
	  //	  p.address = sender_address; // compatibility requirement
	  dlink_recv_queue.push(p);
	  pthread_mutex_unlock(&dlink_recv_queue_mutex);
	  sem_post(&dlink_recv_queue_has_data);
	  // send ack for the packet
	  pthread_mutex_lock(&ack_send_queue_mutex);
	  dbglog( "Sending ack for address: " << (int)sender_address 
		  << " with recv seq # == send seq # == " <<  (unsigned)recv_window[(int)sender_address].NFE);

	  ack_send_queue.push( Ack(sender_address,seq_num) );
	  pthread_mutex_unlock(&ack_send_queue_mutex);
	  // invert NFE for next cycle
	  recv_window[(int)sender_address].NFE = ( recv_window[(int)sender_address].NFE )?0:1;
	}else{
	  dbglog("[Ack seq # doesn't match... dropping packet ( NFE = "
		 << (int)recv_window[(int)sender_address].NFE << ").");
	  dbglog("[But sending ack for last packet again...");
	  dbglog("[Sending ack for address: " << (int)sender_address 
		 << " with seq #" <<  (int)seq_num);
	  pthread_mutex_lock(&ack_send_queue_mutex);
	  ack_send_queue.push( Ack(sender_address,seq_num) );
	  pthread_mutex_unlock(&ack_send_queue_mutex);
	  
	  // pthread_mutex_lock(&link_recv_queue_mutex;)
	  //	  continue;
	}
	// send the queued acks
	flush_acks();
      } else {
	dbglog("Control frame received from " << (int)sender_address );
	bk_sctrl(dummy_seq,dummy_PF_bit,seq_num,control);
	
	pthread_mutex_lock(&ack_recv_queue_mutex);
	ack_recv_queue.push( Ack(p.address,seq_num) );
	pthread_mutex_unlock(&ack_recv_queue_mutex);
	sem_post(&ack_recv_queue_has_data);
      }
      pthread_mutex_lock(&link_recv_queue_mutex);
    } // !queue empty
    pthread_mutex_unlock(&link_recv_queue_mutex);

  } // while !quit
  
  fout("datalink_recv_element()");
  pthread_exit(0);
}

void* dlink_layer(void* arg) 
{
  fin("dlink_layer()");
  pthread_t tid;
  
  // initialize layer
  init_dlink_layer();
  
  // spawn receive element
  int rv = pthread_create(&tid,NULL,dlink_recv_element,NULL);
  checkval_0(rv,Exception("link_layer()","could not spawn receive element"));
  pthread_detach(tid);
  
  // spawn sending element
  rv = pthread_create(&tid,NULL,dlink_send_element,NULL);
  checkval_0(rv,Exception("link_layer()","could not spawn sending element"));
  pthread_detach(tid);

  fout("dlink_layer()");
  pthread_exit(0);
}


// extra fragments

      
//       dbglog( " [ Length = " << p.frame.size() << endl;
//       dbglog( " [ Control = " << (unsigned)control << endl;
//       dbglog( " [ From Address = " << ((int)sender.address) << endl;
//       dbglog( " [ Frame = " << p.frame << endl;
      
//       // deframe and update recv window information
//        dbglog( "Frame Type is: "
// 	   << (is_ictrl(control)?"Informatory":"Supervisory") << endl;

