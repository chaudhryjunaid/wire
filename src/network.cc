//
// Copyright 2015 Chaudhry Junaid Anwar
//


#define DEBUG_THIS_FILE
#include "debug.hh"
#include <string>
#include <list>
#include <vector>
#include <queue>
#include <errno.h>
#include "threads.hh"
#include "datalink.hh"
#include "fileparser.hh"
#include "externals.hh"
using namespace std;

typedef packet Packet_t;

queue<Packet_t> nl_send_queue;
queue<Packet_t> nl_recv_queue;

pthread_mutex_t nl_send_queue_mutex;
pthread_mutex_t nl_recv_queue_mutex;

sem_t nl_send_queue_has_data;
sem_t nl_recv_queue_has_data;

struct linkinfo
{
  linkinfo(unsigned n,unsigned w):neigh(n),weigh(w)
  {}

  unsigned neigh;
  unsigned weigh;
};

struct lsp
{
  lsp(unsigned a,unsigned b,unsigned c):dest(a),weigh(b),nhop(c)
  {}

  unsigned dest;
  unsigned weigh;
  unsigned nhop;
};


// graph data
vector < list<linkinfo> > v;
pthread_mutex_t graph_mutex;
vector< lsp > cls;
pthread_mutex_t cls_mutex;



vector< list<linkinfo> > adjmat(File_parser fp)
{
  list<linkinfo> ls;
  vector < list<linkinfo> > v;
  string s;
  
  cout<<"\nCreating Adjacency Matrix."<<endl;
  
  while(1)
    {
      //      cout<<"Pehla"<<endl;
      if((s=fp.get_next_token())!=string())
	{
	  while(1)
	    {
	      string a=fp.get_next_token();
	      if(a!=".")
		{
		  string b=fp.get_next_token();

		  int aa=atoi(a.c_str());
		  int bb=atoi(b.c_str());

		  ls.push_back(linkinfo(aa,bb));
		}
	      else
		{
		  break;
		}
	    }
	  v.push_back(ls);
	  ls.erase(ls.begin(),ls.end());
	}
      else
	{
	  break;
	}
    }
  cout<<"Adjacency Matrix Created."<<endl;
  return v;
}


void printgraph()
{
  cout<<"\nPrint Link State Lists."<<endl;
  pthread_mutex_lock(&graph_mutex);
  for (unsigned i=0;i< v.size(); i++)
    {
      cout << "Beginning list # " << i << endl;
    list<linkinfo>::iterator li;
    li  = v[i].begin();
    while(li != v[i].end())
      {
	cout << "Dest = " << li->neigh << endl;
	cout << "Wieght = " << li->weigh << endl;
	li++;
      }
    }
  pthread_mutex_unlock(&graph_mutex);
}



unsigned calcnh(vector< lsp > cls, unsigned last, unsigned node, unsigned first)
{
  //  cout<<"\nEntering the function:";
  unsigned head=last;
  unsigned tail=first;

  // cout<<"tail is "<<tail<<endl;
  // cout<<"head is "<<head<<endl;
  // cout<<"Entering the loop:";
  
  while(head!=tail)
    {
      //  cout << "pappo yaar tang na kar" << endl;
     
      for(unsigned i=0;i<cls.size();i++)
	{
	  if(tail==cls[i].dest) 
	    {
	      //            cout << "TAIL = " << tail << endl;
	      //    cout << "head = " << head << endl;
	      head=tail;
	      tail=cls[i].nhop;
	    }
	  if(head==tail)
	    {
	      return head;
	    }
	}
    }
    
  return head;
}



vector< lsp > dijkistra(unsigned node)
{
  // nobody should use graph during dijkistra
  pthread_mutex_lock(&graph_mutex);
  cout<<"\nGenerating Routing table."<<endl;
  unsigned enter=0;
  vector < lsp > cls;
  vector < lsp > tls;
  list < linkinfo >::iterator li;
  unsigned next_hop;

  //  cout<<"Tls size : "<<tls.size()<<endl;
  //  cout<<"Cls size : "<<cls.size()<<endl;

  cls.push_back(lsp(node,0,node));
  
  while(1){
    
  unsigned last;
  unsigned first;

  last=cls[cls.size()-1].dest;
  first=cls[cls.size()-1].nhop;
  //  cout<<"dest is now :"<<last<<endl;

  for(li = v[last].begin();li!= v[last].end();li++)
    {

      //  cout<<"Next hop is"<<next_hop<<endl;
      int c=0;
      for(unsigned i=0;i<tls.size();i++)
	{
	  if (tls.empty())
	    break;

	  //    cout<<"tls is : "<<tls.size()<<endl;
	  // cout<<"i is : "<<i<<endl;
	  if(tls[i].dest==li->neigh)
	  {
	    //	    int pp=li->weigh+cls[cls.size()-1].weigh;
	    //	    cout<<"Weight is :"<<pp<<endl;
	    if((li->weigh+cls[cls.size()-1].weigh)<tls[i].weigh)
	    {
	      //cls[cls.size()-1].dest
	      //cls[cls.size()-1].nhop
	      c=1;
	      next_hop=calcnh(cls,last,node,first);
	      tls[i]=lsp(li->neigh,li->weigh+cls[cls.size()-1].weigh,next_hop);
	    }
	  }
	}
      
      int m=0;
      for(unsigned l=0;l<cls.size();l++)
	{
	  if(li->neigh==cls[l].dest)
	    {
	      m=1;
	      break;
	    }
	}

      if((cls.size()>1)&&(m==0)&&(c==0))
	{
	  //	  cout<<"More than 1"<<endl;
	  //	  cout<<"Weight is :"<<cls[cls.size()-1].weigh+li->weigh<<endl;
	  next_hop=calcnh(cls,last,node,first);     
	  tls.push_back(lsp(li->neigh,cls[cls.size()-1].weigh+li->weigh,next_hop));
	}

      else if((cls.size()==1)&&(m==0))
	{
	  //	  cout<<"Only one in cls"<<endl;
	  tls.push_back(lsp(li->neigh,li->weigh,li->neigh));
	}
    }
  
  //  cout<<"Tentive list is "<<endl;
  for (unsigned su=0;su<tls.size();su++)
    {
      //  cout<<"("<<tls[su].dest<<tls[su].weigh<<tls[su].nhop<<")"<<endl;
    }

  //  cout<<"Tls size : "<<tls.size()<<endl;

  unsigned min;
  unsigned min_idx;

  if(!tls.empty())
    {
      min=tls[0].weigh;
      min_idx=0;
    
      for (unsigned j=0;j<tls.size();j++)
	{
	  //  cout<<"tls min ="<<endl;
	  if((tls[j].weigh)<min)
	    {
	      min=tls[j].weigh;
	      min_idx=j;
	    }
	}

      int yup=0;
      int pup=0;
      for(unsigned d=0;d<cls.size();d++)
	{
	  if(cls[d].dest==tls[min_idx].dest)
	    {
	      //  cout<<"main hoon na"<<endl;
	      yup=1;
	      if(cls[d].weigh>tls[min_idx].dest)
		{
		  pup=1;
		  cls.push_back(tls[min_idx]);
		}
	      break;
	    }
	}
      if(!yup&&!pup)
	cls.push_back(tls[min_idx]);
      
      tls.erase(tls.begin()+min_idx);
	
    }

  //  cout<<"\nClist is : "<<endl;

  if(cls.size()==v.size())
    break;

  enter++;
  if(enter==v.size())
    break;
  }

  cout<<"Routing table done."<<endl;

  pthread_mutex_unlock(&graph_mutex);
  return cls;

}


void my_state()
{
  pthread_mutex_lock(&cls_mutex);
  pthread_mutex_lock(&graph_mutex);

  cout<<"\nChecking state of my node"<<endl;
  if(cls.size()==1)
    {
      cout<<"Oops, I have lost my connection with all"<<endl;
    }
  else if(cls.size()==v.size())
    {
      cout<<"MashAllah! I can reach all the nodes."<<endl;
    }
  else
    {
      cout<<"Somebody Poor is Out of Order and Unreachable"<<endl;
    }

  cout<<"My Routing Table is: "<<endl;
  cout<<"\tDestiny\tWeight\tNext_hop"<<endl;
  for(unsigned k=0;k<cls.size();k++)
    {
      cout<<"\t(  "<<cls[k].dest<<" \t  "
	  <<cls[k].weigh<<" \t   "<<cls[k].nhop<<"   )"<<endl;
    }
  pthread_mutex_unlock(&graph_mutex);
  pthread_mutex_unlock(&cls_mutex);
}

void print_string(string s)
{
  cout<<"Packet: "<<endl;
  for(unsigned i=0;i<s.size();i++)
    {
      if ( i<4 ){
	cout<<(int)s[i] << "  ";
      }
      cout<<(char)s[i];
    }
  cout<<endl;
}

void create_lsp(unsigned node,string &s,vector<list <linkinfo> > v,char &s_s_num)
{
  cout<<"\nCreating Lsp:"<<endl;
  unsigned lsp_idx=2;
  unsigned age=16;
  list <linkinfo>::iterator li;
  for(li=v[node].begin();li!=v[node].end();li++)
    {
      s.push_back(((char)li->neigh));
      s.push_back(((char)li->weigh));
    }
  
  s.insert(s.begin(),1,(char)age);
  s.insert(s.begin(),1,s_s_num);
  s.insert(s.begin(),1,(char)node);
  s.insert(s.begin(),1,(char)lsp_idx);

  //  print_string(s);

  s_s_num++;
  cout<<"Lsp succesfully created"<<endl;
}

void create_hello(unsigned node,string &s)
{
  unsigned hello_idx=1;
  cout<<"\nCreating the Hello Packet"<<endl;
  s.push_back((char)hello_idx);
  s.push_back((char)node);
  
  //  print_string(s);
  cout<<"Hello Packet Creation Successful\n";
}

void  create_data(unsigned node,string &s,string  payload,unsigned dest)
{
  cout<<"\nCreating Data Packet"<<endl;
  unsigned data_idx=0;
  unsigned age=16;
  s.push_back((char)data_idx);
  s.push_back((char)node);
  s.push_back((char)dest);
  s.push_back((char)age);

  s+=payload;
  cout<<"Data Packet Created"<<endl;
}

void  read_packet(unsigned &r_node,string &pack_recv,unsigned &r_dest,char &r_s_num,unsigned &type,string &pay_recv,string &r_lsp, unsigned &r_age)
{
  type=(unsigned)pack_recv[0];
  r_node=(unsigned)pack_recv[1];

  if(type==1)
    {
      return;
    }
  else if(type==0)
    {
      r_age=(unsigned)pack_recv[3];
      r_dest=(unsigned)pack_recv[2];
      pay_recv=pack_recv.substr(4,(pack_recv.length()-1));
    }
  else if(type==2)
    {      
      r_age=(unsigned)pack_recv[3];
      r_s_num=(unsigned)pack_recv[2];
      r_lsp=pack_recv.substr(4,pack_recv.length()-1);
    }
}
void  con_strto_list(string r_lsp,list <linkinfo> &li)
{
  unsigned i=0;
  cout<<"Start The conversion"<<endl;
  while(i<r_lsp.length()-1)
    {
      cout<<"Lovely"<<endl;
      li.push_back(linkinfo((unsigned)r_lsp[i],(unsigned)r_lsp[i+1]));
      i++;i++;
    }
  list <linkinfo>::iterator p;
  for(p=li.begin();p!=li.end();p++)
    {
      cout<<p->neigh<<"\t "<<p->weigh<<"\t";
    }
  cout<<"\nNew List being cretaed"<<endl;

}

unsigned update_graph(string &all_seq,unsigned r_node, string r_lsp,char r_s_num)
{
  cout<<"Updating the Graph"<<endl;
  list <linkinfo> li;
  unsigned i=0;
  //print_string(all_seq);
  if(r_s_num>all_seq[r_node])
    {
      all_seq[r_node]=r_s_num;
      con_strto_list(r_lsp,li);
      pthread_mutex_lock(&graph_mutex);
      v[r_node]=li;
      pthread_mutex_unlock(&graph_mutex);
      printgraph();
      i=1;
    }
  if(i)
    cout<<"Updation Done"<<endl;
  else
    cout<<"No update required"<<endl;

  return i;
}

void print_myqueue(queue <string> q)
{
  string s;
  int b;
  cout<<"\nPrinting the queue"<<endl;
  while(q.size()>0)
    {
      s=q.front();
      b=atoi(s.c_str());
      cout<<s<<endl;
      q.pop();
    }
}


void enq_pack(string r_data,char r_prevhop=0)
{
  pthread_mutex_lock(&cls_mutex);
  if(r_data[0]==0)
    {
      pthread_mutex_lock(&dlink_send_queue_mutex);
      dlink_send_queue.push(packet((char)cls[(unsigned)r_data[2]].nhop,r_data));
      pthread_mutex_unlock(&dlink_send_queue_mutex);
      sem_post(&dlink_send_queue_has_data);
    }
  else if (r_data[0]==2)
    {
      for(unsigned i=0;i<cls.size();i++)
	{
	  if(i!=(unsigned)r_prevhop)
	      {
		pthread_mutex_lock(&dlink_send_queue_mutex);
		dlink_send_queue.push(packet((char)i,r_data));
		pthread_mutex_unlock(&dlink_send_queue_mutex);
		sem_post(&dlink_send_queue_has_data);
	      }
	}
    }
  pthread_mutex_unlock(&cls_mutex);
}


void init_network_layer(void)
{
  fin("init_network_layer()");

  File_parser tp("topology.txt");

  v=adjmat(tp); // create initial routing table from file
  printgraph(); // print to verify

 
  cls = dijkistra(ext::my_address);

  //  my_state (cls,v);


  // initialize global variables
  checkval_0(sem_init(&nl_send_queue_has_data,0,0),
	     Exception("init_nl_layer","semaphore could not be initialized."));
  checkval_0(sem_init(&nl_recv_queue_has_data,0,0),
	     Exception("init_nl_layer","semaphore could not be initialized."));

  checkval_0(pthread_mutex_init( &nl_send_queue_mutex,NULL),
	     Exception("init_nl_layer","mutex could not be initialized.") );
  checkval_0(pthread_mutex_init( &nl_recv_queue_mutex,NULL),
	     Exception("init_nl_layer","mutex could not be initialized.") );


  checkval_0(pthread_mutex_init( &graph_mutex,NULL),
	     Exception("init_nl_layer","mutex could not be initialized.") );
  checkval_0(pthread_mutex_init( &cls_mutex,NULL),
	     Exception("init_nl_layer","mutex could not be initialized.") );


  fout("init_network_layer()");
}

void* network_recv_element(void*)
{
    fin("network_recv_element()");

    string all_seq;
    for (unsigned i=0;i<v.size();i++)
      all_seq.push_back(0);

    packet r_packet;
    char r_prevhop;
    char r_s_num;
    unsigned r_dest;
    unsigned r_node;
    string r_data;
    string r_pay;
    string r_lsp;
    unsigned type;
    unsigned r_age;
    unsigned up_req=0;

    while (!ext::time_to_quit){

      sem_wait(&dlink_recv_queue_has_data);
      pthread_mutex_lock(&dlink_recv_queue_mutex);
      while( !dlink_recv_queue.empty() ){

	r_packet = dlink_recv_queue.front();
	dlink_recv_queue.pop();
	pthread_mutex_unlock(&dlink_recv_queue_mutex);

	r_data = r_packet.frame;
	r_prevhop = r_packet.address;

	read_packet(r_node,r_data,r_dest,r_s_num,type,r_pay,r_lsp,r_age);
	cout<<"recv node is"<<r_node<<endl;
	cout<<"type is:"<<type<<endl;
	if(type==0)
	  {
	    cout<<"Destination is :"<<r_dest<<endl;
	    print_string(r_pay);
	    if(r_dest==(unsigned)ext::my_address)
	      {
		cout<<"Move to Application layer"<<endl;
		pthread_mutex_lock(&nl_recv_queue_mutex);
		nl_recv_queue.push(packet(r_node,r_pay));
		pthread_mutex_unlock(&nl_recv_queue_mutex);
		sem_post(&nl_recv_queue_has_data);
	      }
	    else
	      {
		cout<<"Move the packet to the send side"<<endl;
		enq_pack(r_data);
	      }  
	  }
	if(type==2)
	  {
	    cout<<"Sequence Number is :"<<(unsigned)r_s_num<<endl;
	    //	  print_string(r_lsp);
	    if(r_node!=(unsigned)ext::my_address && r_age>=0)
	      {
		up_req=update_graph(all_seq,r_node,r_lsp,r_s_num);
	 
		if(up_req)
		  {
		    cls=dijkistra(ext::my_address);
		    // my_state(cls,v);
		  }
		r_data[3]--;
		if((unsigned)(r_data[3])>0)
		  {
		    cout<<"Move to send Queue"<<endl;
		    enq_pack(r_data,r_prevhop);
		  }
	      }
	  }
	pthread_mutex_unlock(&dlink_recv_queue_mutex);
      }
    }

    fout("network_recv_element()"); 
    pthread_exit(0);
}

void* network_send_element(void*)
{
    fin("network_send_element()");

    string my_data = "";
    string my_pay = "";
    packet p;

    while (!ext::time_to_quit){

      sem_wait(&nl_send_queue_has_data);
      pthread_mutex_lock(&nl_send_queue_mutex);

      while(!nl_send_queue.empty())
	{
	  p = nl_send_queue.front();
	  nl_send_queue.pop();
	  pthread_mutex_unlock(&nl_send_queue_mutex);

	  cout << "Packet received." << endl
	       << "Payload: " << p.frame <<endl
	       << "Address: " << (int)p.address <<endl;

	  create_data(ext::my_address,my_data,p.frame,(unsigned char)p.address);
	  
	  enq_pack(my_data);
	  cout << "Encapsulated packet: " << my_data << endl;
	  print_string(my_data);
	  my_data.clear();
	  pthread_mutex_lock(&nl_send_queue_mutex);
	}

      pthread_mutex_unlock(&nl_send_queue_mutex);
    }

    fout("network_send_element()");
    pthread_exit(0);
}


void* network_layer(void* arg) 
{
  fin("network_layer()");
  cout << "Network layer active." << endl;
  pthread_t tid;
  
  // initialize layer
  init_network_layer();
  
  // spawn receive element
  int rv = pthread_create(&tid,NULL,network_recv_element,NULL);
  checkval_0(rv,Exception("link_layer()","could not spawn receive element"));
  pthread_detach(tid);
  
  // spawn sending element
  rv = pthread_create(&tid,NULL,network_send_element,NULL);
  checkval_0(rv,Exception("link_layer()","could not spawn sending element"));
  pthread_detach(tid);

  fout("network_layer()");
  pthread_exit(0);
}
