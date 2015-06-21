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
