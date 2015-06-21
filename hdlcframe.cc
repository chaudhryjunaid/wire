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

// 
// hdlcframe.cc
// class for hdlc framing and fragmentation of arbitrary length buffers
// *********************************************************************
// 

#include <iostream>
#include "debug.hh"
#include "hdlcframe.hh"
#include "crc32.hh"
using namespace std;

//#define DEBUG_THIS_FILE

void printbits(long l)
{
  cout << "The bit sequence is: ";
  for(int i=0; i<32; i++){
    cout << (bool) (( l  & (0x80000000>>i)  )?1:0);
    if ( (i+1)%4 == 0) cout <<  " "; 
  }
  cout << endl;
}

short mk_ictrl(char N_Send,bool PF_bit, char N_Recv)
{
  control_field cf;
  cf.bits.if_.if_sf = 0;
  cf.bits.if_.N_Send = N_Send;
  cf.bits.if_.PF_bit = PF_bit;
  cf.bits.if_.N_Recv = N_Recv;
  return cf.bytes;
}
// Sfnc_bits: 0,1,2,3
// N_Recv: 7-bit seq no.
short mk_sctrl(char Sfnc_bits,bool PF_bit,char N_Recv)
{
  control_field cf;
  cf.bits.sf_.if_sf = 1;
  cf.bits.sf_.zero_bit = 0;
  cf.bits.sf_.Sfnc_bits = Sfnc_bits;
  cf.bits.sf_.zero_bits = 0;
  cf.bits.sf_.PF_bit = PF_bit;
  cf.bits.sf_.N_Recv = N_Recv;
  return cf.bytes;
}

bool is_ictrl(short ctrl)
{
  
  control_field cf;
  cf.bytes = ctrl;
  return !cf.bits.if_.if_sf;	// return true if information frame's ctrl field
}

short bk_ictrl(char& N_Send,bool& PF_bit,char& N_Recv,short ctrl)
{
  control_field cf;
  cf.bytes = ctrl;
  N_Send = cf.bits.if_.N_Send;
  N_Recv = cf.bits.if_.N_Recv;
  PF_bit = cf.bits.if_.PF_bit;
  return ctrl;
}

short bk_sctrl(char& Sfnc_bits,bool& PF_bit,char& N_Recv,short ctrl)
{
  control_field cf;
  cf.bytes = ctrl;
  N_Recv = cf.bits.sf_.N_Recv;
  PF_bit = cf.bits.sf_.PF_bit;
  Sfnc_bits = cf.bits.sf_.Sfnc_bits;
  return ctrl;
}

string s_to_b(const string& s)
{
  fin("s_to_b()");

  string bs;			// string for holding bits

  for (unsigned i=0;i<s.size();i++){
    bs += getbits(s[i]);
  }
  
//   dbgcode(
// 	  dbglog("bitstring: ");
// 	  for (unsigned j=0; j<bs.size();j++){
// 	    if (j%8==0) ' ')dlog<< ' ';
// 	  dlog <<  (unsigned)bs[j] );
// 	  }
// 	  dbglog("");
//       );
  
  fout("s_to_b()");
  return bs;
}

string b_to_s(const string& bs)
{
  fin("b_to_s()");
  char ch = 0;
  string s;
  unsigned i;

  for(i=1; i<=bs.size(); i++){
    ch<<=0x1;
    if (bs[i-1]){
      ch|=0x1;			// set lsb
    }
    if(i%8 == 0){
      // cout << "[" << i/8 << "] ";
      // printbits(ch);
      s.push_back(ch);
    }
  }

  // pad extra bits with zeros
  if(--i%8){
    ch<<=(8-i%8);
    s.push_back(ch);
  }

  fout("b_to_s()");
  return s;
}

string append_fcs(string& s)
{
  Crc slowCrc(eth_key);		// make crc object
  s.resize (s.length () + 4);	// resize string to accomodate fcs
  
  for (unsigned i = 0; i < s.length (); ++i){
    slowCrc.put_byte (s[i]);
  }

  // add crc to the lower four bytes of s (currently 0)
  ulong crc = slowCrc.done ();
  char crc_str[4];
  memcpy(crc_str,&crc,4);
  s[s.length()-1] = crc_str[0];
  s[s.length()-2] = crc_str[1];
  s[s.length()-3] = crc_str[2];
  s[s.length()-4] = crc_str[3];
  
  // verify by calculating crc again
  dbgcode(
	  for (unsigned d = 0; d < s.length(); ++d){
	    slowCrc.put_byte (s[d]);
	  }
	  ulong zero = slowCrc.done();
	  dbglog("Remainder after appending crc: " << zero);
	  assert(!zero); 	// remainder should be zero
	  );

  return s;			// return completed frame_load
}

// returns 0 if frame crc is correct
// strips the crc part of the frame
int check_fcs(string& s)
{
  Crc slowCrc(eth_key);		// make crc object
  for (unsigned i = 0; i < s.length(); ++i){
    slowCrc.put_byte (s[i]);
  }
  ulong remainder = slowCrc.done();

  s.resize (s.length () - 4);	// resize string to delete fcs
  //  s.erase(s.length()-4);	// delete the last four bytes of the string
  dbglog("Checkfcs(): CRC check = " << remainder);
  return remainder;		// should always be zero for a correct frame
}

string complete_frame(string s)
{
  string frame;
  frame.push_back(flag);

  s = s_to_b(s);

  dbglog(s);
  char bits_set = 0;
  for(unsigned i=0; i<s.size(); i++){
    if (s[i]){
      bits_set++;
    }else{
      bits_set = 0;
    }
    if (bits_set==5){
      s.insert(s.begin()+i+1,1,0);
      bits_set = 0;
    }
  }
  dbglog("completeframe(): Frame after bitstuffing = ");
  dbglog(s);

  s = b_to_s(s);
  frame+=s;

  frame.push_back(flag);

  return frame;
}

string
frame(
      const char addr,	     // address of recv or send station
      const short control,   // take from frame modules mk_Xctrl functions
      const char* info,      // pre-allocated buffer containing networklayer packet
      const int  info_size   // total bytes to be sent from info, set to zero for S-frame
      )
{
  string payload;

  for(int i=0; i<info_size; i++){
    payload.push_back( *(info+i) );
  }

  string frameload;
  // always push low byte of ctrl onto frame first and pop in same order
  frameload.push_back(addr);
  frameload.push_back( *( (char*) &control) );
  frameload.push_back( *( ((char*)&control) + 1 ));
  frameload += payload;

  append_fcs(frameload);

  //  dbglog("CRCframe = " << frameload);
  // do bitstuffing and include flags
  string frame = complete_frame(frameload);

  dbgcode(
	  //  dbglog( "frame() debug output..." );
	  //  dbglog( "The frameload was: " << frameload);
	  //  dbglog( "with frameload size: " << frameload.size() )      ;
	  dbglog( "frame(): the frame is: "<< endl << frame);
	  dbglog( "  with size: " << frame.length() );
	  );

  return frame;
}

int				// returns frame status byte
deframe(
	char& addr,		// address field copied here
	short& control,		// control field copied here
	string& frame		// pass frame as bitstring, here
	)
{
  fin("deframe()");

  // first and last bytes are preassumed to be start and stop flags
  //  dbglog( "deframe(): frame length = " << frame.length() );
  //  dbglog( "The frame is: " << frame );
  frame.erase(0,1);		// remove start flag
  frame.erase(frame.length()-1,1);	// remove end flag
  //  dbglog( "deframe(): flags removed... " << frame );

  frame = s_to_b(frame);

  char bits_set = 0;
  for(unsigned i=0; i<frame.size(); i++){
    if (frame[i]){
      bits_set++;
    }else{
      bits_set = 0;
    }
    if (bits_set==5){
      frame.erase(frame.begin()+i+1);
      bits_set = 0;
    }
  }

  // dbglog( "after un-bitstuffing, frame=" << frame );

  frame = b_to_s(frame);
  if (frame[frame.size()-1]==0){
    frame.erase(frame.length()-1,1);	// remove end byte if 0 (due to padding)
  }

  //  dbglog("after compacting size = " << frame.size() );
 
  if (check_fcs(frame)){
    dbglog( "Frame received incorrectly.");
    return 1;			// indicate fcs error
  }
  
  addr = frame[0];
  *( (char*) &control ) = frame[1];
  *( ((char*)&control) + 1) = frame[2];

  // remove header bytes
  frame = frame.c_str()+3;

  dbgcode(dbglog("deframe(): Deframing successful with:  " << frame ););

  fout("deframe()");
  return 0;
}
