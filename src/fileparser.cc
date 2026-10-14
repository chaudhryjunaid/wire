//
// Copyright 2015 Chaudhry Junaid Anwar
//


#include <cstdlib>
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
    cout << "File open operation failed: " << filename << endl;
    cout << "It is opened relative to the current directory." << endl;
    exit(1);
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
  // if previous was the last string of file
  // return empty string
  if (idx>=file_str.size()){
    return string();
  }

  // the last token has no trailing separator when the file does not
  // end in whitespace (find() + 1 used to wrap npos round to 0 here)
  string::size_type end = file_str.find(' ',idx);
  if (end==string::npos){
    end = file_str.size();
  }

  string token = file_str.substr(idx,end-idx);
  idx = end + 1;
  return token;
}

unsigned File_parser::get_len()
{
  return file_str.length();
}

