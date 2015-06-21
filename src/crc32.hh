// crc32 class
// calculates crc32 string for any string of length n+4 where the last four bytes are 0
// 
// a constant eth_key can be used as a generating polynomial
// 
// copied from a website, not created by authors
// if you are the owner of this code, please write to us so that your copyright notice can be added
//

#ifndef CRC32_HH_
#define CRC32_HH_


typedef unsigned long ulong;
static ulong const eth_key = 0x04c11db7;

class Crc
{
public:
  Crc (ulong key) : _key (key), _register (0)
  {}
  void put_byte (unsigned char byte);
  ulong done ()			// return crc
  {
    ulong tmp = _register;
    _register = 0;
    return tmp;
  }
private:
  void put_bit (bool bit);
  ulong _key;	// really 33-bit key, counting implicit 1 top-bit
  ulong _register;
};


#endif	// CRC32_HH_
