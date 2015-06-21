//
// Copyright 2015 Chaudhry Junaid Anwar
//


// 
// simple log class
// needs mutex protection in multithreaded environments
// 
// 

#ifndef LOG_CLASS_HH_
#define LOG_CLASS_HH_


#include <fstream>
#include <string>
#include <ctime>
#include <cassert>
using namespace std;

class Log
{
public:
  Log(const char* log_file_name = "templog", char* log_header=0, bool append_time=false);
  virtual ~Log();

  void write(char* s = "log written");
  void end(char* footer = "Logging stopped");

  ostream& filestream(void);
private:
  ofstream logstream_;
  bool timestamps_enabled_;
  time_t t_;
};

#endif	// LOG_CLASS_HH_
