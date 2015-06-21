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

// debug.hh
// version 3.0
// logging enabled
// =============================================================================
// To use FILE_TRACE_VERSION, declare following in main module
//   Log tlog("program.trace.log",0,true);
// Similarly, to use FILE_DEBUG_VERSION, declare the following
//    Log dlog("program.debug.log",0,true);
// 


// 
// contains macros
// 
// dbgcode() - executes code enclosed only if DEBUG_VERSION is defined
// dbglog() - prints log to stdout if DEBUG_VERSION is defined or logfile if FILE_DEBUG_VERSION is defined
// fin() - prints a trace message, function entering
// fout() - prints a trace message, function exitting
// 


#ifndef DEBUG_MACS_H_
#define DEBUG_MACS_H_


#include <iostream>
#include <fstream>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cassert>
#include "log.hh"		// for logging support
#include "externals.hh"

using namespace std;

#define DBG_INDICATOR " [dbg] "


// dbglog(log) definition
#ifdef FILE_DEBUG_VERSION

 #ifdef DEBUG_THIS_FILE
  #define dbglog(log) { dlog.filestream() << DBG_INDICATOR << log << endl; }
 #else
  #define dbglog(log) ;
 #endif

#else

 #ifdef DEBUG_VERSION
  #ifdef DEBUG_THIS_FILE
   #define dbglog(log) { cout << log << endl; }
  #else
   #define  dbglog(log) ;
  #endif
 #else // !DEBUG_VERSION
  #define dbglog(log) ;
 #endif

#endif

// use dbglog() in code instead of cout 

#ifdef DEBUG_VERSION

 #ifdef DEBUG_THIS_FILE
   #define dbgcode(code) { code; }
 #else
   #define dbgcode(code) ;
 #endif

#else

 #define dbgcode(code) ;

#endif

// trace macros

// fin(func)
#ifdef STD_TRACE_VERSION

 #ifdef DEBUG_THIS_FILE
   #define fin(fname) { cout << "[0m Entering function " << fname << "[0m" << endl; }
 #else
   #define fin(fname) ;
 #endif

#else

 #ifdef FILE_TRACE_VERSION
   #ifdef DEBUG_THIS_FILE
    #define fin(fname) { tlog.filestream() << " (+) Entering function [   " << fname << "   ]" << endl; }
   #else
    #define fin(fname) ;
   #endif

 #else

   #define fin(fname) ;
 #endif

#endif

// fout()
#ifdef STD_TRACE_VERSION

  #ifdef DEBUG_THIS_FILE
   #define fout(fname) { cout << "\t Leaving function " << fname << " -----" << endl; }
  #else
   #define fout(fname) ;
  #endif

#else

 #ifdef FILE_TRACE_VERSION

   #ifdef DEBUG_THIS_FILE
    #define fout(fname) { tlog.filestream() << " (-) Leaving function [   " << fname << "   ]" << endl; }
   #else
    #define fout(fname) ;
   #endif

 #else
   #define fout(fname) ;
 #endif

#endif



#endif	// end DEBUG_MACS_H_
