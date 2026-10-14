//
// Copyright 2015 Chaudhry Junaid Anwar
//


#include <iostream>
#include <sstream>
#include <string>

#include "externals.hh"
#include "threads.hh"
#include "client.hh"
#include "link.hh"
#include "udp.hh"
#include "datalink.hh"
#include "network.hh"

using namespace std;
using namespace ext;


// prints the messages the network layer delivers to this node
void* app_recv_element(void*)
{
  while (!ext::time_to_quit){
    sem_wait(&nl_recv_queue_has_data);
    pthread_mutex_lock(&nl_recv_queue_mutex);
    while (!nl_recv_queue.empty()){
      packet p(nl_recv_queue.front());
      nl_recv_queue.pop();
      Out_line() << "CLIENT: [data received from node " << (int)p.address << "]: "
	   << p.frame << endl;
    }
    pthread_mutex_unlock(&nl_recv_queue_mutex);
  }
  pthread_exit(0);
}

int main(int argc, char* argv[])
{
  fin("main");
  set_terminate(report_uncaught_exception);	// report errors from the layer threads
  srand(time(NULL) ^ getpid());	// nodes started in the same second must not share fault patterns
  try {
    Out_line() << "Entering try block..." << endl;
    pthread_t tid;
    if (argc!=2 || atoi(argv[1])<0 || atoi(argv[1])>=ext::total_nodes){
      Out_line() << "usage: " << argv[0] << " <my_node_id 0-" << (int)ext::total_nodes-1 << ">" << endl
	   << "then type one message per line as: <dest_node_id> <message>" << endl;
      exit(1);
    }

    ext::packet_err_wt = 15;
    ext::packet_drop_wt = 15;
    ext::my_address = atoi(argv[1]);

    pthread_create(&tid,NULL,link_layer,(void*)(long)atoi(argv[1]));
    pthread_join(tid,NULL);
    Out_line() << "Physical layer created." << endl;

    pthread_create(&tid,NULL,dlink_layer,NULL);
    pthread_join(tid,NULL);
    Out_line() << "Datalink created" << endl;

    pthread_create(&tid,NULL,network_layer,NULL);
    pthread_join(tid,NULL);
    Out_line() << "Network layer created." << endl;
    sleep(1);

    pthread_create(&tid,NULL,app_recv_element,NULL);
    Out_line() << "Node " << (int)ext::my_address << " ready. Send with: <dest_node_id> <message>" << endl;

    string line;
    while (getline(cin,line)){
      istringstream fields(line);
      int dest;
      packet a;

      if (!(fields >> dest)){
	if (line.find_first_not_of(" \t")!=string::npos){
	  Out_line() << "Send with: <dest_node_id> <message>" << endl;
	}
	continue;		// blank lines are ignored
      }
      fields >> ws;
      getline(fields,a.frame);
      if (dest<0 || dest>=ext::total_nodes || a.frame.empty()){
	Out_line() << "Send with: <dest_node_id 0-" << (int)ext::total_nodes-1 << "> <message>" << endl;
	continue;
      }

      a.address = dest;
      pthread_mutex_lock(&nl_send_queue_mutex);
      nl_send_queue.push(a);
      pthread_mutex_unlock(&nl_send_queue_mutex);
      sem_post(&nl_send_queue_has_data);
    }

    // other nodes' routes may pass through this one, so keep the stack
    // running after input ends rather than exiting
    Out_line() << "End of input: still forwarding and receiving. Press Ctrl-C to quit." << endl;
    pthread_join(tid,NULL);
  }

  catch(Errno_exception ee){
    ee.printerr();
    exit(1);
  }
  catch(int errno_thrown) {
    Out_line() << "syscall throwed errno " << errno_thrown << endl;
    perror("Client");
    exit(1);
  }
  catch(char* s){
    Out_line() << "string exception caught: " << s << endl;
    herror("h_errno state");
    exit(1);
  }
  catch(...){
    Out_line() << "Caught default exception" << endl;
    throw ;
  }

  fout("main");
  return 0;
}
