//
// Copyright 2015 Chaudhry Junaid Anwar
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

