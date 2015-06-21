//
// Copyright 2015 Chaudhry Junaid Anwar
//


#include <iostream>

#include "externals.hh"
#include "threads.hh"
#include "client.hh"
#include "link.hh"
#include "udp.hh"
#include "datalink.hh"
#include "network.hh"

using namespace std;
using namespace ext;


int main(int argc, char* argv[])
{
  fin("main");
  srand(time(NULL));
  try {
    cout << "Entering try block..." << endl;
    pthread_t tid; 
    if (argc<3){
      exit(1);
    }
    
    ext::packet_err_wt = 15;
    ext::packet_drop_wt = 15;
    ext::my_address = atoi(argv[1]);

    pthread_create(&tid,NULL,link_layer,(void*)atoi(argv[1]));    
    pthread_join(tid,NULL);
    cout << "Physical layer created." << endl;
    
    pthread_create(&tid,NULL,dlink_layer,NULL);
    pthread_join(tid,NULL);
    cout << "Datalink created" << endl;
    
    pthread_create(&tid,NULL,network_layer,NULL);
    pthread_join(tid,NULL);
    cout << "Network layer created." << endl;
    sleep(1);


    packet a;
    a.address = atoi(argv[2]);
    
    for (int i=0; i< 10;i++){
      if (atoi(argv[1])==0){
	//      cout<< "Enter a string: ";
	cin >> a.frame;
	pthread_mutex_lock(&nl_send_queue_mutex);
	nl_send_queue.push(a);
	pthread_mutex_unlock(&nl_send_queue_mutex);
	sem_post(&nl_send_queue_has_data);
      }else{
	sem_wait(&nl_recv_queue_has_data);
	pthread_mutex_lock(&nl_recv_queue_mutex);
	cout << "CLIENT: [data received]: "
	     << dlink_recv_queue.front().frame << endl;
	nl_recv_queue.pop();
	pthread_mutex_unlock(&nl_recv_queue_mutex);
      }
    }

  cout << "Going to sleep." << endl;
  sleep(2);
  }
  
  catch(Errno_exception ee){
    ee.printerr();
    exit(1);
  }
  catch(int errno_thrown) {
    cout << "syscall throwed errno " << errno_thrown << endl;
    perror("Client");
    exit(1);
  }
  catch(char* s){
    cout << "string exception caught: " << s << endl;
    herror("h_errno state");
    exit(1);
  }
  catch(...){
    cout << "Caught default exception" << endl; 
    throw ;
  }

  fout("main");
  return 0;
}
