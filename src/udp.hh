//
// Copyright 2015 Chaudhry Junaid Anwar
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
#include <unistd.h>

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

