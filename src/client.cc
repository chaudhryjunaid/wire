//
// Copyright 2015 Chaudhry Junaid Anwar
//

#include <iostream>
#include <getopt.h>
#include "externals.hh"
#include "threads.hh"
#include "client.hh"
#include "link.hh"
#include "udp.hh"
#include "datalink.hh"


using namespace std;
using namespace ext;


int main(int argc, char* argv[])
{
  fin("main");


// int next_option;
// /* A string listing valid short options letters. */
// const char* const short_options = “ho:v”;
// /* An array describing valid long options. */
// const struct option long_options[] = {
// { “help”, 0, NULL, ‘h’ },
// { “output”, 1, NULL, ‘o’ },
// { “verbose”, 0, NULL, ‘v’ },
// { NULL, 0, NULL, 0 } /* Required at end of array. */
// };
// /* The name of the file to receive program output, or NULL for
// standard output. */
// const char* output_filename = NULL;
// /* Whether to display verbose messages. */
// int verbose = 0;
// /* Remember the name of the program, to incorporate in messages.
// The name is stored in argv[0]. */
// program_name = argv[0];
// do {
// next_option = getopt_long (argc, argv, short_options,
// long_options, NULL);
// switch (next_option)
// {
// case ‘h’: /* -h or --help */
// /* User has requested usage information. Print it to standard
// output, and exit with exit code zero (normal termination). */
// print_usage (stdout, 0);
// case ‘o’: /* -o or --output */
// /* This option takes an argument, the name of the output file. */
// output_filename = optarg;
// break;
// case ‘v’: /* -v or --verbose */
// verbose = 1;
// break;
// case ‘?’: /* The user specified an invalid option. */
// /* Print usage information to standard error, and exit with exit
// code one (indicating abnormal termination). */
// print_usage (stderr, 1);
// case -1: /* Done with options. */
// break;
// default: /* Something else: unexpected. */
// abort ();
// }
// }
// while (next_option != -1);

// if (verbose) {
// int i;
// for (i = optind; i < argc; ++i)
// printf (“Argument: %s\n”, argv[i]);
// }
// /* The main program goes here. */

  if (argc>=4) ext::total_nodes = atoi(argv[3]);
  ext::my_address = atoi(argv[1]);
  packet a;
  a.address = atoi(argv[2]);
  
  ext::packet_drop_wt = 1;
  ext::packet_err_wt = 1;

  try {
    cout << "Entering try block..." << endl;
    pthread_t tid; 
    if (argc>3){
      pthread_create(&tid,NULL,link_layer,(void*)atoi(argv[1]));
    }else{
      cout << "args" << endl;
      exit(1);
    }
    
    pthread_join(tid,NULL);
    pthread_create(&tid,NULL,dlink_layer,NULL);
    pthread_join(tid,NULL);
    cout << "Datalink created" << endl;

    char x[200];

    while(1) {
      if (atoi(argv[1])==0){
	cout<< "------------------- " << endl;
	cin.getline(x,200);
	a.frame = x;
	pthread_mutex_lock(&dlink_send_queue_mutex);
	dlink_send_queue.push(a);
	pthread_mutex_unlock(&dlink_send_queue_mutex);
	sem_post(&dlink_send_queue_has_data);
      }else{
	sem_wait(&dlink_recv_queue_has_data); 
	pthread_mutex_lock(&dlink_recv_queue_mutex);
	cout << "*** CLIENT: [data received]: "
	     << dlink_recv_queue.front().frame << endl;
	dlink_recv_queue.pop();
	pthread_mutex_unlock(&dlink_recv_queue_mutex);
      }
    }

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
