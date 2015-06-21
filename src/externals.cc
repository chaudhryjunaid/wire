//
// Copyright 2015 Chaudhry Junaid Anwar
//


// externals.cc
// defines all global variables within namespace ext

#include "externals.hh"
#include "log.hh"

#ifdef FILE_DEBUG_VERSION
Log tlog("program.trace.log",0,true);
Log dlog("program.debug.log",0,true);
#endif

namespace ext {
  bool time_to_quit = false;

  char total_nodes = 5;		// default topology
  char my_address = 0;		// default is server

  char packet_drop_wt = 0;	// rand()%packet_drop_wt
  char packet_err_wt = 0; 	// rand()%packet_err_wt

  int max_drop_idx = 300;	// err will always be introduced in first max_drop_idx bytes

  int max_timeouts = 5;		// max tries on timeout
  //  clock_t to_val = 20000;	// default timeout value
  timeval to_val = {0,1000000};	// has to be a large enough number
}
