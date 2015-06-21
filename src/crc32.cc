// class that calculates crc32 for set of bytes
// adapted version from online example
// copied from a website, not created by authors

#include <cassert>
#include <iostream>
#include <string>
#include "crc32.hh"


void Crc::put_byte (unsigned char byte)
{
  unsigned char mask = 0x80; // leftmost bit
  for (int j = 0; j < 8; ++j)
    {
      put_bit ((byte & mask) != 0);
      mask >>= 1;
    }
}

void Crc::put_bit (bool bit)
{
  bool top_bit = (_register & 0x80000000) != 0;
  // shift bit into register from the right
  _register <<= 1;
  _register ^= (bit? 0x1: 0x0); // OR or XOR, same result
  if (top_bit)
    {
      // XOR(subtract) the 32-bits of the key.
      // The implicit high bit of the 33-bit key
      // conceptually clears the topBit shifted out of the register
      _register ^= _key;
    }
}

/* class usage
***************

int main ()
{
  std::string msg ("Harry had a little lamp");
  std::string exMsg (msg);
  exMsg.resize (msg.length () + 4);
  
  Crc slowCrc (ethernetKey);
  
  for (size_t i = 0; i < exMsg.length (); ++i)
  {
  slowCrc.put_byte (exMsg [i]);
  }
  ulong crc = slowCrc.done ();
  std::cout << "\n0x" << std::hex << crc << std::endl;

  return 0;
}
*/
