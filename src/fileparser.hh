//
// Copyright 2015 Chaudhry Junaid Anwar
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
