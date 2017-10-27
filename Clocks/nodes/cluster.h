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
#include <limits.h>
#include <condition_variable>

#include <list>
#include "logger.h"
using namespace std;

typedef struct node_config {
	int           nc_id;
	int           nc_port_num;
	string        nc_ip_addr;
	unsigned long nc_clock;
} node_config_t;

typedef struct cluster_config {
	int t_daemon;
	list<node_config_t *> cc_cluster;
} cluster_config_t;

node_config_t *cc_get_record(int id, cluster_config_t *cc);
int insert_node_config(cluster_config_t *cc, node_config_t *nc);
list<node_config_t *> get_list(cluster_config_t *cc);
void elect_coordinator(cluster_config_t *cc);
node_config_t *cc_get_daemon(cluster_config_t *cc);
int load_cluster(int id, char *configi, cluster_config_t *cc);
#endif
