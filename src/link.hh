//
// Copyright 2015 Chaudhry Junaid Anwar
//


#ifndef LINK_H_
#define LINK_H_


void init_physical_layer(int arg);
void* link_layer(void* arg) ;
char* get_buffer(string s);

struct dgram
{
  char address;
  string frame;
  dgram()
  {}
  dgram(char addr,string f):address(addr),frame(f)
  {}
};


#endif	// LINK_H_
