#ifndef __MULT_H_
#define __MULT_H_
#include "cluster.h"
void multicast_init_vector(node_status_t *ns);
void multicast(node_status_t *ns);
void multicast_ready(node_status_t *ns, int id);
void mulicast_recv(node_status_t *ns, unsigned long msg[10], int id);
#endif
