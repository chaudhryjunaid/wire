//
// Copyright 2006, 2007 Ch. Junaid Anwar, Syed Usman Ali, M. Shahzad Mughal, Saira Zahid.
//
// This file is part of NG-DS (or) NGDS.
//
// NG-DS is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// Foobar is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with Foobar.  If not, see <http://www.gnu.org/licenses/>.
//
// For any addition information, contact:
//  chjunaidanwar@gmail.com
//
// This program was compiled edited with Emacs and compiled with GCC
// using the GNU make system
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
