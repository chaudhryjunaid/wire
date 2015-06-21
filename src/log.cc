//
// Copyright 2015 Chaudhry Junaid Anwar
//


#include "log.hh"

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

void Log::write(char* s)
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

void Log::end(char* footer)
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
