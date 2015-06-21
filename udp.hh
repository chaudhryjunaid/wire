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
// NG-DS is distributed in the hope that it will be useful,
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

// class usage notes
// =================
// (1) construct the object, call bind_socket for server and fix_dest for client,
//     and send or recv w/o giving dests.
// (2) otherwise just construct the object and call recv_dgram/send_dgram
// 
// ==============================================================================

#ifndef UDP_SOCKETAPI_WRAPPER__
#define UDP_SOCKETAPI_WRAPPER__


#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>

class Udpsocket
{
public:
  Udpsocket();			// calls socket() to get socket descriptor
  ~Udpsocket();			// calls close() on socket descriptor

  // bind socket to local address for receiving datagrams
  int bind_socket(const char *host, const int port, const bool host_is_addr = false);
  // fix the destination to send datagrams to 
  int fix_dest(const char *dest, const int port, const bool dest_is_addr = false);

  // communication routines
  int send_dgram(const void *msg, const int len,const unsigned flags = 0);
  int recv_dgram(void *buf, const int len,const unsigned flags = 0);

  // functions
  int get_sockfd(void) const;
  char* get_ip(void) const;
  char* get_name(void) const;
  int get_port(void) const;
  hostent* get_he(void) const;

  // helper functions
  void fill_host(const in_addr addr, const int port);

private:
  int sockfd_;
  sockaddr_in host_addr_;	// can be local or remote host
  bool host_set_;
};



#endif	// end UDP_SOCKETAPI_WRAPPER__

