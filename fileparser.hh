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

// class `config file parser`
// class for parsing configuration & startup files
// =================================================


// 
// 
// # in the file starts an end-of-line-comment
// comments and empty lines are ignored
// each line contains `fields` separated by tabs or spaces
// spaces preceded by a `\` are not counted as separators
// 

// 
// only text files are accepted
// the file must be a unix text file
// 

#ifndef FILE_PARSER_HH_
#define FILE_PARSER_HH_

#include <iostream>
#include <fstream>
#include <string>
#include <list>
using namespace std;

class File_parser
{
public:
  File_parser(const string& filename);
  virtual ~File_parser();

  unsigned get_len(); // return the total length of internal string
  string get_next_token(void); // return the next word
private:
  string file_str;
  unsigned idx;
};

#endif	// FILE_PARSER_HH_
