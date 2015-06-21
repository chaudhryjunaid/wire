//
// Copyright 2015 Chaudhry Junaid Anwar
//


// all global vars are declared here
// and defined in externals.cc
// include externals.hh wherever you want to use these variables

#include "debug.hh"
#include "error.hh"
#include "link.hh"
#include "udp.hh"
#include "threads.hh"
#include <queue>

#ifdef FILE_DEBUG_VERSION
extern Log tlog;
extern Log dlog;
#define dout dlog
#else
#define dout ;
#endif

class Exception;
extern Exception def_except;


// link layer
extern queue<dgram> link_send_queue;
extern queue<dgram> link_recv_queue;
extern pthread_mutex_t link_send_queue_mutex;
extern pthread_mutex_t link_recv_queue_mutex;
extern sem_t link_send_queue_has_data;
extern sem_t link_recv_queue_has_data;

// datalink layer

typedef dgram packet;

extern queue<packet> dlink_send_queue;
extern queue<packet> dlink_recv_queue;
extern pthread_mutex_t dlink_send_queue_mutex;
extern pthread_mutex_t dlink_recv_queue_mutex;
extern sem_t dlink_send_queue_has_data;
extern sem_t dlink_recv_queue_has_data;

extern queue<packet> nl_send_queue;
extern queue<packet> nl_recv_queue;

extern pthread_mutex_t nl_send_queue_mutex;
extern pthread_mutex_t nl_recv_queue_mutex;

extern sem_t nl_send_queue_has_data;
extern sem_t nl_recv_queue_has_data;


namespace ext {
  extern bool time_to_quit;
  extern char total_nodes;
  extern char my_address;
  extern char packet_drop_wt;
  extern char packet_err_wt;
  extern int  max_drop_idx;
  extern int  max_timeouts;
  extern timeval to_val;
}
