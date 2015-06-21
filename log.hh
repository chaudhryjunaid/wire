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
// NG-DS is distributed in the hope that it will be useful,
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
