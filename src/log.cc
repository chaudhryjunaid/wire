//
// Copyright 2015 Chaudhry Junaid Anwar
//


#include <iostream>
#include <pthread.h>
#include "log.hh"

static pthread_mutex_t out_line_mutex = PTHREAD_MUTEX_INITIALIZER;

Out_line::~Out_line()
{
  pthread_mutex_lock(&out_line_mutex);
  cout << buf_.str() << flush;
  pthread_mutex_unlock(&out_line_mutex);
}

string printable(const string& bytes)
{
  static const char hex_digits[] = "0123456789abcdef";
  string s;
  for (unsigned i=0;i<bytes.size();i++){
    unsigned char c = bytes[i];
    if (c>=0x20 && c<0x7f && c!='\\'){
      s.push_back(c);
    }else{
      s += "\\x";
      s.push_back(hex_digits[c>>4]);
      s.push_back(hex_digits[c&0xf]);
    }
  }
  return s;
}

Log::Log(const char* log_file_name, char* log_header, bool append_time)
{
  assert(log_file_name);
  logstream_.open(log_file_name,ios::app);

  if (!logstream_){
    throw -1;
  }

  if(log_header){
    logstream_ << log_header << endl;
  } else {
    t_ = time(0);
    logstream_ << "\t=========================================================" << endl ;
    logstream_ << "\t LOGGING STARTED (at) " << ctime(&t_);
    logstream_ << "\t=========================================================" << endl ;
  }

  timestamps_enabled_ = append_time;
}

Log::~Log()
{
  logstream_.close();
}

void Log::write(const char* s)
{
  assert(logstream_);
  if (timestamps_enabled_){
    char current_time[10];
    t_ = time(0);
    strftime( current_time, sizeof(current_time), "%T", localtime(&t_) );
    logstream_ << "( " <<  current_time << " ): ";
  }
  if (s){
  logstream_ << s << endl;
  } else {
    logstream_ << "<null>" << endl;
  }
}

void Log::end(const char* footer)
{
  if (footer) {
    logstream_ << footer << endl;
  }
  logstream_.close();
}

ostream& Log::filestream(void)
{
  return logstream_;
}
