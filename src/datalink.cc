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
  bool reset;			// acknowledges an SF_RESET, not an I-frame
  Ack(char addr,Seq_num_t seq,bool rst=false):address(addr),seq_num(seq),reset(rst)
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
  bool needs_reset;		// receiver's expected seq num unknown: send SF_RESET first
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
    short control= mk_sctrl(send_ack.reset?SF_RESET_ACK:SF_ACK,0,send_ack.seq_num);
    framed_packet.frame = frame(framed_packet.address,control,framed_packet.frame.data(),framed_packet.frame.size() );
      
    dbglog("flush_acks(): Sending ack: " << printable(framed_packet.frame) << "  to  " << (int)framed_packet.address );
      
    pthread_mutex_lock(&link_send_queue_mutex);
    link_send_queue.push( framed_packet );
    pthread_mutex_unlock(&link_send_queue_mutex);
    sem_post(&link_send_queue_has_data);
  }
  pthread_mutex_unlock(&ack_send_queue_mutex);

  fout("flush_acks()");
}

// Sends a framed packet to neighbour dest and waits for its acknowledgement
// (an SF_RESET_ACK if reset is true, otherwise an SF_ACK), resending on
// every timeout. Returns false after ext::max_timeouts tries.
bool send_until_acked(int dest,Send_packet& sp,bool reset)
{
  int timeouts = 0;
  while (timeouts<ext::max_timeouts){
    pthread_mutex_lock(&link_send_queue_mutex);
    gettimeofday(&sp.send_time,0);
    link_send_queue.push(sp.p);
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

	if (a.address==dest && a.seq_num==sp.seq_num && a.reset==reset){
	  dbglog( "Ack sequence number matched.");
	  dbglog( "Acknowlegement accepted.");
	  return true;
	}
	dbglog( "Ack received but sender, sequence number or type does not match.");
      }else if ( timeout(sp) ){
	break;
      }
    } // while ack arrives, wait

    timeouts++;
    dbglog( "Timeout occured " << timeouts << " time.");
  }
  return false;
}

void* dlink_send_element(void* arg)
{
  fin("datalink_send_element()");

  Packet_array_t link[ext::total_nodes];
  Send_window_state_t send_window[ext::total_nodes];

  // initialize send windows' states
  for (int i=0;i<ext::total_nodes;i++){ // run all sliding window protocols sequentially
    send_window[i].LFS = 1;	//start swp from 0
    send_window[i].needs_reset = true; // the receiver may hold state from an earlier run
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
      if (a_pack.address<0 || a_pack.address>=ext::total_nodes){
	dbglog("Packet addressed to unknown node " << (int)a_pack.address << ", dropping it.");
	continue;
      }
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

	// sequence number for the next packet
	Seq_num_t next = send_window[i].LFS ? 0 : 1;

	// fetch packet from queue front (dequeue from original)
	packet framed_packet = link[i].front();
	link[i].erase(link[i].begin());

	// Before the first frame on a link, and after giving up on one, we
	// can't know which sequence number the receiver expects: it may or
	// may not have the abandoned frame, or may have restarted. Guessing
	// wrong makes it ack and discard a new frame as a duplicate, so tell
	// it which number comes next and wait until it confirms.
	if (send_window[i].needs_reset){
	  packet reset_packet;
	  reset_packet.address = i;
	  reset_packet.frame = "[#]"; // cannot allow zero byte payload currently
	  reset_packet.frame = frame(reset_packet.address,mk_sctrl(SF_RESET,0,next),reset_packet.frame.data(),reset_packet.frame.size() );
	  Send_packet reset_sp(reset_packet,next);
	  dbglog("Resetting link to " << i << ": next sequence number is " << (int)next);
	  if (!send_until_acked(i,reset_sp,true)){
	    dbglog( "Destination node unreachable.");
	    break;
	  }
	  send_window[i].needs_reset = false;
	}

	// frame the packet and store as current
	send_window[i].LFS = next;
	short control= mk_ictrl(0,0,send_window[i].LFS);
	framed_packet.frame = frame(framed_packet.address,control,framed_packet.frame.data(),framed_packet.frame.size() );
	send_window[i].sent_packet = Send_packet(framed_packet, send_window[i].LFS);

	if (!send_until_acked(i,send_window[i].sent_packet,false)){
	  dbglog( "Destination node unreachable.");
	  send_window[i].needs_reset = true;
	  break; //maybe notify network layer in future versions
	}
	
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
      if (sender_address<0 || sender_address>=ext::total_nodes){
	dbglog("Frame from unknown node: dropping packet.");
	pthread_mutex_lock(&link_recv_queue_mutex);
	continue;
      }
      if ( deframe(p.address,control,p.frame) ) {
	dbglog("Cyclic Redundency Check error: dropping packet.");
	pthread_mutex_lock(&link_recv_queue_mutex);
	continue; 		// go for next frame
      }
      // deframe() stored the frame's address field (our own address) in
      // p.address; the layers above and the ack matching need the sender
      p.address = sender_address;

      if (is_ictrl(control)) {
	dbglog("Information frame received from " << (int)sender_address);
	// if information frame, send to above layer
	bk_ictrl(dummy_seq,dummy_PF_bit,seq_num,control);
	if (  seq_num == recv_window[(int)sender_address].NFE  ){
	  pthread_mutex_lock(&dlink_recv_queue_mutex);
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
	char sfnc;
	bk_sctrl(sfnc,dummy_PF_bit,seq_num,control);

	if (sfnc==SF_RESET){
	  // the sender can't tell which frame we expect, so it tells us
	  dbglog("Link reset by " << (int)sender_address << ": next sequence number is " << (int)seq_num);
	  recv_window[(int)sender_address].NFE = seq_num;
	  pthread_mutex_lock(&ack_send_queue_mutex);
	  ack_send_queue.push( Ack(sender_address,seq_num,true) );
	  pthread_mutex_unlock(&ack_send_queue_mutex);
	  flush_acks();
	}else{
	  pthread_mutex_lock(&ack_recv_queue_mutex);
	  ack_recv_queue.push( Ack(p.address,seq_num,sfnc==SF_RESET_ACK) );
	  pthread_mutex_unlock(&ack_recv_queue_mutex);
	  sem_post(&ack_recv_queue_has_data);
	}
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

