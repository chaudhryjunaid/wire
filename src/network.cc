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
#include <climits>
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
  
  Out_line() <<"\nCreating Adjacency Matrix."<<endl;
  
  while(1)
    {
      //      Out_line() <<"Pehla"<<endl;
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
  Out_line() <<"Adjacency Matrix Created."<<endl;
  return v;
}


void printgraph()
{
  Out_line() <<"\nPrint Link State Lists."<<endl;
  pthread_mutex_lock(&graph_mutex);
  for (unsigned i=0;i< v.size(); i++)
    {
      Out_line() << "Beginning list # " << i << endl;
    list<linkinfo>::iterator li;
    li  = v[i].begin();
    while(li != v[i].end())
      {
	Out_line() << "Dest = " << li->neigh << endl;
	Out_line() << "Wieght = " << li->weigh << endl;
	li++;
      }
    }
  pthread_mutex_unlock(&graph_mutex);
}



// returns the routing table for node: one {dest, weigh, nhop} entry per
// reachable node, in the order they were settled (node itself first), so
// look entries up by dest rather than indexing by node id
vector< lsp > dijkistra(unsigned node)
{
  // nobody should use graph during dijkistra
  pthread_mutex_lock(&graph_mutex);
  Out_line() <<"\nGenerating Routing table."<<endl;

  const unsigned unreached = UINT_MAX;
  vector < unsigned > dist(v.size(),unreached);
  vector < unsigned > nhop(v.size(),node);
  vector < bool > settled(v.size(),false);
  vector < lsp > cls;
  list < linkinfo >::iterator li;

  if (node<v.size())
    dist[node]=0;

  while(1){
    // settle the closest node not yet in the routing table
    unsigned u=unreached;
    for (unsigned i=0;i<v.size();i++)
      if (!settled[i] && dist[i]!=unreached && (u==unreached || dist[i]<dist[u]))
	u=i;
    if (u==unreached)
      break;			// everything left is unreachable

    settled[u]=true;
    cls.push_back(lsp(u,dist[u],nhop[u]));

    for(li = v[u].begin();li!= v[u].end();li++)
      {
	if (li->neigh>=v.size() || settled[li->neigh])
	  continue;
	if (dist[u]+li->weigh<dist[li->neigh])
	  {
	    dist[li->neigh]=dist[u]+li->weigh;
	    // our own neighbours are their own next hop; anything further
	    // away is reached through the same first hop as u
	    nhop[li->neigh]=(u==node)?li->neigh:nhop[u];
	  }
      }
  }

  Out_line() <<"Routing table done."<<endl;

  pthread_mutex_unlock(&graph_mutex);
  return cls;

}


void my_state()
{
  pthread_mutex_lock(&cls_mutex);
  pthread_mutex_lock(&graph_mutex);

  Out_line() <<"\nChecking state of my node"<<endl;
  if(cls.size()==1)
    {
      Out_line() <<"Oops, I have lost my connection with all"<<endl;
    }
  else if(cls.size()==v.size())
    {
      Out_line() <<"MashAllah! I can reach all the nodes."<<endl;
    }
  else
    {
      Out_line() <<"Somebody Poor is Out of Order and Unreachable"<<endl;
    }

  Out_line() <<"My Routing Table is: "<<endl;
  Out_line() <<"\tDestiny\tWeight\tNext_hop"<<endl;
  for(unsigned k=0;k<cls.size();k++)
    {
      Out_line() <<"\t(  "<<cls[k].dest<<" \t  "
	  <<cls[k].weigh<<" \t   "<<cls[k].nhop<<"   )"<<endl;
    }
  pthread_mutex_unlock(&graph_mutex);
  pthread_mutex_unlock(&cls_mutex);
}

// prints a packet: its 4 header bytes as numbers, then its payload
void print_string(string s)
{
  Out_line line;
  line<<"Packet: header";
  for(unsigned i=0;i<s.size() && i<4;i++)
    {
      line<<" "<<(int)(unsigned char)s[i];
    }
  if (s.size()>4)
    {
      line<<", payload: "<<printable(s.substr(4));
    }
  line<<endl;
}

void create_lsp(unsigned node,string &s,vector<list <linkinfo> > v,char &s_s_num)
{
  Out_line() <<"\nCreating Lsp:"<<endl;
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
  Out_line() <<"Lsp succesfully created"<<endl;
}

void create_hello(unsigned node,string &s)
{
  unsigned hello_idx=1;
  Out_line() <<"\nCreating the Hello Packet"<<endl;
  s.push_back((char)hello_idx);
  s.push_back((char)node);
  
  //  print_string(s);
  Out_line() <<"Hello Packet Creation Successful\n";
}

void  create_data(unsigned node,string &s,string  payload,unsigned dest)
{
  Out_line() <<"\nCreating Data Packet"<<endl;
  unsigned data_idx=0;
  unsigned age=16;
  s.push_back((char)data_idx);
  s.push_back((char)node);
  s.push_back((char)dest);
  s.push_back((char)age);

  s+=payload;
  Out_line() <<"Data Packet Created"<<endl;
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
  Out_line() <<"Start The conversion"<<endl;
  while(i<r_lsp.length()-1)
    {
      Out_line() <<"Lovely"<<endl;
      li.push_back(linkinfo((unsigned)r_lsp[i],(unsigned)r_lsp[i+1]));
      i++;i++;
    }
  list <linkinfo>::iterator p;
  for(p=li.begin();p!=li.end();p++)
    {
      Out_line() <<p->neigh<<"\t "<<p->weigh<<"\t";
    }
  Out_line() <<"\nNew List being cretaed"<<endl;

}

unsigned update_graph(string &all_seq,unsigned r_node, string r_lsp,char r_s_num)
{
  Out_line() <<"Updating the Graph"<<endl;
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
    Out_line() <<"Updation Done"<<endl;
  else
    Out_line() <<"No update required"<<endl;

  return i;
}

void print_myqueue(queue <string> q)
{
  string s;
  int b;
  Out_line() <<"\nPrinting the queue"<<endl;
  while(q.size()>0)
    {
      s=q.front();
      b=atoi(s.c_str());
      Out_line() <<s<<endl;
      q.pop();
    }
}


void enq_pack(string r_data,char r_prevhop=0)
{
  pthread_mutex_lock(&cls_mutex);
  if(r_data[0]==0)
    {
      // cls is in settle order, not indexed by node id: find dest's entry
      unsigned dest=(unsigned char)r_data[2];
      unsigned k=0;
      while (k<cls.size() && cls[k].dest!=dest)
	k++;
      if (k==cls.size())
	{
	  Out_line() <<"No route to node "<<dest<<", dropping packet."<<endl;
	}
      else
	{
	  pthread_mutex_lock(&dlink_send_queue_mutex);
	  dlink_send_queue.push(packet((char)cls[k].nhop,r_data));
	  pthread_mutex_unlock(&dlink_send_queue_mutex);
	  sem_post(&dlink_send_queue_has_data);
	}
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

  // printgraph() and dijkistra() below lock these, so set them up first
  checkval_0(pthread_mutex_init( &graph_mutex,NULL),
	     Exception("init_nl_layer","mutex could not be initialized.") );
  checkval_0(pthread_mutex_init( &cls_mutex,NULL),
	     Exception("init_nl_layer","mutex could not be initialized.") );

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
	Out_line() <<"recv node is"<<r_node<<endl;
	Out_line() <<"type is:"<<type<<endl;
	if(type==0)
	  {
	    Out_line() <<"Destination is :"<<r_dest<<endl;
	    print_string(r_data);
	    if(r_dest==(unsigned)ext::my_address)
	      {
		Out_line() <<"Move to Application layer"<<endl;
		pthread_mutex_lock(&nl_recv_queue_mutex);
		nl_recv_queue.push(packet(r_node,r_pay));
		pthread_mutex_unlock(&nl_recv_queue_mutex);
		sem_post(&nl_recv_queue_has_data);
	      }
	    else
	      {
		// age works like the IP TTL: one less per forward, and a
		// packet that runs out is dropped, so a routing loop (nodes
		// with different topology files, say) can't keep it forever
		unsigned char age=(unsigned char)r_data[3];
		if (age<=1)
		  {
		    Out_line() <<"Packet from node "<<r_node<<" to node "<<r_dest
			       <<" ran out of age, dropping it."<<endl;
		  }
		else
		  {
		    r_data[3]=(char)(age-1);
		    Out_line() <<"Move the packet to the send side"<<endl;
		    enq_pack(r_data);
		  }
	      }
	  }
	if(type==2)
	  {
	    Out_line() <<"Sequence Number is :"<<(unsigned)r_s_num<<endl;
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
		    Out_line() <<"Move to send Queue"<<endl;
		    enq_pack(r_data,r_prevhop);
		  }
	      }
	  }
	pthread_mutex_lock(&dlink_recv_queue_mutex);
      }
      pthread_mutex_unlock(&dlink_recv_queue_mutex);
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

	  Out_line() << "Packet received." << endl
	       << "Payload: " << printable(p.frame) <<endl
	       << "Address: " << (int)p.address <<endl;

	  create_data(ext::my_address,my_data,p.frame,(unsigned char)p.address);
	  
	  enq_pack(my_data);
	  Out_line() << "Encapsulated packet: " << printable(my_data) << endl;
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
  Out_line() << "Network layer active." << endl;
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
