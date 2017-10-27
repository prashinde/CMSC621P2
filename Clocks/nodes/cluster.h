#ifndef __CL_H_
#define __CL_H_

#include <thread>
#include <queue>
#include <mutex>
#include <iostream>
#include <fstream>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <condition_variable>

#include <list>
#include "logger.h"
using namespace std;

typedef struct node_config {
	char nc_ip_addr[16];
	int  nc_port_num;
} node_config_t;

typedef struct cluster_config {
	list<node_config_t *> cc_cluster;
} cluster_config_t;

int insert(cluster_config_t *cc, node_config_t *nc);
list<node_config_t *> get_list(cluster_config_t *cc);
int load_cluster(int id, char *config);
#endif
