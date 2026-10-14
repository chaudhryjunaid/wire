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
#include <sstream>
#include <string>
#include <ctime>
#include <cassert>
using namespace std;

class Log
{
public:
  Log(const char* log_file_name = "templog", char* log_header=0, bool append_time=false);
  virtual ~Log();

  void write(const char* s = "log written");
  void end(const char* footer = "Logging stopped");

  ostream& filestream(void);
private:
  ofstream logstream_;
  bool timestamps_enabled_;
  time_t t_;
};

// Collects one statement's output and writes it to stdout in one piece
// when the statement ends, so lines printed by different threads never
// get mixed together. Use it wherever you would write to cout:
//   Out_line() << "x = " << x << endl;
class Out_line
{
public:
  ~Out_line();			// writes the collected text, under a mutex

  template<typename T>
  Out_line& operator<<(const T& value)
  {
    buf_ << value;
    return *this;
  }
  Out_line& operator<<(ostream& (*manip)(ostream&)) // endl, flush, ...
  {
    buf_ << manip;
    return *this;
  }

private:
  ostringstream buf_;
};

// returns bytes with anything unprintable shown as \xNN, for printing
// frames and packets without sending raw control bytes to the terminal
string printable(const string& bytes);

#endif	// LOG_CLASS_HH_
