//
// Copyright 2015 Chaudhry Junaid Anwar
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
