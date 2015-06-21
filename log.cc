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
