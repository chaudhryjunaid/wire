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
