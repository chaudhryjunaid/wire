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

#include "error.hh"

Exception def_except("Unkown","Default Exception",MEDIUM);

Exception::Exception(string thrower, string error_string, int severity)
{
  throwing_agent_ = thrower;
  err_msg_ = error_string;
  severity_ = severity;
}

Exception::~Exception()
{
}

void Exception::printerr(void)
{
  cout << throwing_agent_ << ": " << err_msg_ << endl;
}

int Exception::get_severity(void)
{
  return severity_;
}

Errno_exception::Errno_exception(string thrower)
{
  Exception(thrower,strerror(errno),NONFATAL);
  errno_val_ = errno;
}

Errno_exception::~Errno_exception()
{
}

// if errno is set, throw errno exception to indicate errno exception
void check_errno(int ret_val, string calling_function)
{
  if( ret_val == -1 )		// syscall failed
    throw Errno_exception(calling_function);
}

void checkval(int ret_val, int errval, Exception exception)
{
  if( ret_val ==  errval)
    throw exception; 
}

void checkval_good(int ret_val, int goodval, Exception exception)
{
  if( ret_val !=  goodval)
    throw exception; 
}

void checkval_0(int ret_val, Exception exception)
{
  if( ret_val )
    throw exception; 
}
