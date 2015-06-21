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


// hdlcframe.hh
// framing and fragmentation rouine
// 

// 
// if: Information Frame
// sf: Supervisory Frame


#ifndef HDLC_FRAME_FORMAT_H_
#define HDLC_FRAME_FORMAT_H_



#include <string>
#include <cassert>
using namespace std;

const char flag =  0x7e;	//  0111 1110 binary
#define MSB_MASK 0x80		//  1000 0000 binar
#define LSB_MASK 0x01		//  0000 0001 binary

#define BROADCAST 0xf		// 1111 1111 binary


struct if_control
{
  int N_Recv : 7;       	// Recv sequence number
  int PF_bit : 1;       	// Poll/Final bit
  int N_Send : 7;       	// Send sequence number
  int if_sf  : 1;       	// always 0 in an information frame
};

struct sf_control
{
  int N_Recv    : 7;		// Recv sequence number
  int PF_bit    : 1;		// Poll/Final bit
  int zero_bits : 4;		// always 0 (4 bits)
  int Sfnc_bits : 2;    	// Supervisory function bits
  int zero_bit  : 1;		// always 0 (1 bit)
  int if_sf     : 1;		// always 1 in a S-frame
};

union control_bits
{
  if_control if_;
  sf_control sf_;
};

union control_field
{
  short bytes;
  control_bits bits;
};

void printbits(long l);

template<typename t>
string getbits(t val)
{
  string s;
  int bitsize = sizeof(val)*8;

  char ch = 0;
  for(int i=0; i<bitsize; i++){
    ch = (val & ( (1<<(bitsize-1)) >> i) ? 1 : 0 );
    s.push_back( ch );
  }

  return s;
}

string s_to_b(const string& s);
string b_to_s(const string& s);

// pre-conditions:
// N_Send and N_Recv must be 7-bit numbers
// Sfnc_bits must be 0,1,2 or 3 in value
short mk_ictrl(char N_Send,bool PF_bit, char N_Recv);
short mk_sctrl(char Sfnc_bits,bool PF_bit,char N_Recv);
short bk_ictrl(char& N_Send,bool& PF_bit,char& N_Recv,short control);
short bk_sctrl(char& Sfnc_bits,bool& PF_bit,char& N_Recv,short control);
bool is_ictrl(short ctrl);

// crc routines
string append_fcs(string& s);

// returns 0 if frame crc is correct
// strips the crc part of the frame
int check_fcs(string& s);

// usage: use c_str() or data() and cast to void* when transmitting
string
frame(
      const char addr,		// address of recv or send station
      const short control,      // take from frame modules mk_Xctrl functions
      const char* info,	        // pre-allocated buffer containing networklayer packet
      const int info_size       // size of info bufferset to zero for S-frame
      );


int				// returns frame status byte
deframe(
	char& addr,		// address field copied here
	short& control,		// control field copied here
	string& frame		// pass frame as bitstring, here
	);




// // hdlc fields review
// struct HDLCfields
// {
//   char  sflag;
//   char  addr;
//   short ctrl;
//   string info;	// equivalent to a list of chars (remember? info consists of integral octets of bits)
//   long CRC32;
//   char ef;
// };


#endif
