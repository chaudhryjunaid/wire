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

// 
// error.h
// definitions, routines and classes for error handling
// 
// 

#ifndef ERROR_HANDLING_HH_
#define ERROR_HANDLING_HH_



#include <iostream>
#include <errno.h>
#include <string>
#include <cstring>

using namespace std;

// severity levels
#define LOW      0
#define MEDIUM   1
#define NONFATAL 1
#define FATAL    2

// general exception class
class Exception
{
public:
  Exception(string thrower = "Unknown", string error_string = "Unhandled exception",int severity = LOW);
  virtual ~Exception();
  void printerr(void);
  int get_severity(void);
protected:
  string throwing_agent_;
  string err_msg_;
  int severity_;
};

// errno based exception class
class Errno_exception: public Exception
{
public:
  Errno_exception(string thrower = "Unknown");
  virtual ~Errno_exception();

protected:
  int errno_val_;
};


void check_errno(int ret_val, string calling_function);
void checkval(int ret_val, int errval, Exception exception);
void checkval2(int ret_val, int goodval, Exception exception);
void checkval_0(int ret_val, Exception exception);

#endif	// ERROR_HANDLING_HH_
