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

// file summary:-
//   implementation of a wrapper class for udp (socket) api
// ========================================================

//#define DEBUG_THIS_FILE
#include <string.h>
#include "debug.hh"

#include "udp.hh"
#include "error.hh"


Udpsocket::Udpsocket()
{
  fin("Udpsocket::Udpsocket");

  sockfd_ = socket(PF_INET, SOCK_DGRAM, 0);
  check_errno( sockfd_,"Udpsocket::Udpsocket()" );
  dbglog("Socketfd is: " << sockfd_);
  host_set_ = false;

  fout("Udpsocket::Udpsocket");
}

Udpsocket::~Udpsocket()
{
  fin("Udpsocket::~Udpsocket");
  
  int rval = close(sockfd_);
  check_errno( rval,"Udpsocket::~Udpsocket()" );
  
  fout("Udpsocket::~Udpsocket");
}

int Udpsocket::bind_socket(const char *host, const int port, const bool host_is_addr)
{
  fin("Udpsocket::bind_socket");

  hostent* he = 0;

  if (host_is_addr) {
    cout << host_addr_.sin_addr.s_addr << endl; 
    checkval( inet_aton(host,&host_addr_.sin_addr) ,0, Exception("bind_socket","inet_ntoa exception"));
    cout << host_addr_.sin_addr.s_addr << endl; 
    he = gethostbyaddr(&host_addr_.sin_addr,sizeof(in_addr),AF_INET); 
    checkval( (int)he, 0, Exception("bind_socket","gethostbyaddr returned 0") );
    dbglog( "Host name is: " << he->h_name );;
  } else {
    he  = gethostbyname(host);
    checkval ( (int)he, 0, Exception("bind_socket","gethostbyname returned 0"));
    dbglog( "Host address is: " << inet_ntoa( *((in_addr*)he->h_addr) ) );
  }

  fill_host( *(in_addr*)he->h_addr, htons(port));
  host_set_ = true;

  int rval = bind(sockfd_, (sockaddr*)&host_addr_, sizeof(sockaddr));
  check_errno( rval,"bind_socket on bind");
  dbglog( "Socket bind successful." );

  fout("Udpsocket::bind_socket");
  return rval;
}

int Udpsocket::fix_dest(const char *dest, const int port, const bool dest_is_addr)
{
  fin("Udpsocket::fix_dest");

  hostent* he = 0;

  if (dest_is_addr){
    checkval( inet_aton(dest,&host_addr_.sin_addr) ,0, Exception("fix_dest","inet_ntoa exception"));
    he = gethostbyaddr(&host_addr_.sin_addr,sizeof(host_addr_.sin_addr),AF_INET); 
    checkval( (int)he,0,Exception("fix_dest on gethostbyaddr",hstrerror(h_errno) ) );
    dbglog( "Host name is: " << he->h_name);
  } else { 
    he=gethostbyname(dest);
    checkval( (int)he, 0, Exception("fix_dest on gethostbyname",hstrerror(h_errno)) );
    dbglog( "Host address is: " << inet_ntoa( *((in_addr*)he->h_addr) ) );
  }

  fill_host( *(in_addr*)he->h_addr, htons(port) );
  host_set_ = true;

  //  int rval = connect();
  //  return rval;
  fout("Udpsocket::fix_dest");
  return 0; // TODO
}

int Udpsocket::send_dgram(const void *msg, const int len,const unsigned flags)
{
  fin("Udpsocket::send_dgram");

  int rval = sendto(sockfd_,msg,len,0,(sockaddr*)&host_addr_,sizeof(sockaddr));
  check_errno( rval, "send_dgram on sendto");

  dbglog( rval << " bytes sent to host " << inet_ntoa(host_addr_.sin_addr)\
	  << " (port) " <<   ntohs(host_addr_.sin_port) );

  fout("Udpsocket::send_dgram")
  return rval;
}

int Udpsocket::recv_dgram(void *buf, const int len,const unsigned flags)
{
  fin("Udpsocket::recvfrom_");
  socklen_t slen = sizeof(sockaddr);
  int rval = recvfrom(sockfd_,buf,len,0,(sockaddr*)&host_addr_,&slen);
  check_errno( rval,"recv_dgram on recvfrom" );
  dbglog( rval << " bytes form host " << inet_ntoa(host_addr_.sin_addr)\
	  << " (port) " << ntohs(host_addr_.sin_port) );
  fout ("Udpsocket::recvfrom_");
  return rval;
}

int Udpsocket::get_sockfd(void) const
{
  return sockfd_;
}

char *Udpsocket::get_ip(void) const
{
  return inet_ntoa(host_addr_.sin_addr);
}

int Udpsocket::get_port(void) const
{
  return ntohs(host_addr_.sin_port);
}

char* Udpsocket::get_name(void) const
{
 hostent* he = gethostbyaddr(&host_addr_.sin_addr,sizeof(in_addr),AF_INET); 
 checkval( (int)he, 0, Exception("get_name on gethostbyaddr","gethostbyaddr returned 0") );
 return he->h_name;
}

hostent* Udpsocket::get_he(void) const
{
 hostent* he = gethostbyaddr(&host_addr_.sin_addr,sizeof(in_addr),AF_INET); 
 checkval( (int)he, 0, Exception("get_he","gethostbyaddr returned 0") );
 return he;
}

// takes args in NBO
void Udpsocket::fill_host(const in_addr addr, const int port)
{
  host_addr_.sin_family = AF_INET;
  host_addr_.sin_port = port;
  host_addr_.sin_addr.s_addr = addr.s_addr;
  bzero (&(host_addr_.sin_zero), 8);
}

