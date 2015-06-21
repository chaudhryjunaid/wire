//
// Copyright 2015 Chaudhry Junaid Anwar
//


#include <iostream>
#include <cstdlib>
#include "debug.hh"
#include "server.hh"
#include "udp.hh"

using namespace std;


int main(int argc, char* argv[])
{
  fin("main");
  // main try block
  try {
  }
  // catch blocks for the main and nested try blocks
  catch (int errno_thrown) {
    cout << "System call throwed errno with value: " << errno_thrown << endl; 
    exit(0);
  }
  catch(char* s){
    cout << "string exception caught: " << s << endl;
    herror("h_errno state");
    exit(1);
  }

  fout("main");
  return 0;
}
