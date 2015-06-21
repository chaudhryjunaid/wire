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

#include "fileparser.hh"

// **********************************************************************
// to be removed upon receipt
//technical note for U-Bhai
// I have disabled my debug macros
// if you plan to use them,
// you can include debug.hh, error.hh and log.hh and their cpp(cc) files
// and compile the resulting project
// **********************************************************************

File_parser::File_parser(const string& filename)
{
  //  fin("File_parser::File_parser");
  cout << "Entering File_parser() ... " << endl;
  ifstream file(filename.c_str(), ios::in);
  if (!file){
    //    throw Errno_exception("File_parser::File_parser()");
    cout << "File open operation failed!" << endl;
    cout << "Probably the filename was incorrect." << endl;
    //exit(1);
  }

  //  dbglog("Parsing file...");


  char c;
  int line_empty=true;

  while ( file.get(c) ) {
    //    dbglog("current character is: " << c);
    switch(c){
    case '#':
      while ( file.get(c) && c!='\n'){
	//	//	dbglog("inside #");
      }
      file.putback(c);
      break;

    case ' ':
    case '\t':
      if (!line_empty){
	//	//	dbglog("pushing ' ' for first time...");
	file_str.push_back(' ');
      }
      while ( file.get(c) && (c==' ' || c=='\t') ){
	//	//	dbglog("Inside space");
      }
      file.putback(c);
      break;

    case '\n':
      if (!line_empty){
	if (file_str[file_str.size()-1]!=' '){
	  file_str.push_back(' ');
	}
	line_empty = true;
      }
      break;

    default:
      file_str.push_back(c);
      line_empty = false;
    }    
  } // end while

  idx = 0;

  //  dbglog("file parsed as: " << file_str);
  cout << "File parsed as: " << file_str << endl;
  //fout("File_parser::File_parser");
  cout << "Exitting File_parser()... " << endl;
}

File_parser::~File_parser()
{
}

string File_parser::get_next_token(void)
{
  unsigned old_idx = idx;
  idx = file_str.find(' ',idx) + 1;

  // if previous was the last string of file
  // return empty string
  if (idx==string::npos){
    idx = old_idx;
    return string();
  }

  // 
  return file_str.substr(old_idx,idx-old_idx-1);
}

unsigned File_parser::get_len()
{
  return file_str.length();
}

