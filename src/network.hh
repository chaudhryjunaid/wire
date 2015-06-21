//
// Copyright 2015 Chaudhry Junaid Anwar
//


#ifndef NETWORK_H_
#define NETWORK_H_

void* network_layer(void* arg) ;
void* network_send_element(void*);
void* network_recv_element(void*);
void init_network_layer(void);

#endif
