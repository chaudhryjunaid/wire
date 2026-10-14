//
// Copyright 2015 Chaudhry Junaid Anwar
//


#include <exception>
#include <unistd.h>
#include "error.hh"
#include "log.hh"

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
  Out_line() << throwing_agent_ << ": " << err_msg_ << endl;
}

int Exception::get_severity(void)
{
  return severity_;
}

Errno_exception::Errno_exception(string thrower)
  : Exception(thrower,strerror(errno),NONFATAL)
{
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

void report_uncaught_exception(void)
{
  std::exception_ptr uncaught = std::current_exception();
  if (uncaught){
    try {
      std::rethrow_exception(uncaught);
    }
    catch (Exception& e) {
      e.printerr();
    }
    catch (std::exception& e) {
      Out_line() << "Unexpected error: " << e.what() << endl;
    }
    catch (...) {
      Out_line() << "Unexpected error of unknown type." << endl;
    }
  } else {
    Out_line() << "Terminated without an exception." << endl;
  }
  // _exit, not exit: the other threads are still running, and exit()
  // would destroy the global queues and sockets they are using
  _exit(1);
}
