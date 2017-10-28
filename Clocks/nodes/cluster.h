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
#include <list>
#include <condition_variable>

#include "sock.h"
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

/*
 * Node is a state machine. It is in one of the following state.
 * 1. OFF -> Node is not accepting connection.
 * 2. ON -> Node is accepting messages from other nodes.
 * 3. CLK_SYN_START -> Clock synchronization is begin. All message other than
 * essential to clock synchronization are ignored. Everyn node enters this state as soon as
 * they recieve syn message from time daemon.
 * 4. CLK_SYN_UPDATE -> Clock sync in progress time daemon has sent out the updated timestamp.
 *
 * STATE MACHINE: TIME DAEMON
 *
 * OFF------->ON--------->CLOCK_SYN_START
 *           /|\             |
 *            |              |
 *            |              |
 *     CLK_SYN_UPDATE<-------                           
 * */

enum node_states {
	OFF,
	ON,
	CLK_SYN_START,
	CLK_SYN_UPDATE
};

typedef struct node_status {
	bool              ns_isdmon;
	enum node_states  ns_state;
	node_config_t    *ns_self;
	c_sock            ns_server;
	cluster_config_t *ns_cc;
} node_status_t;

typedef struct node_con_ctx {
	node_status_t *ncc_ns;
	c_sock        *ncc_cs;
} node_con_ctx_t;

node_config_t *cc_get_record(int id, cluster_config_t *cc);
int insert_node_config(cluster_config_t *cc, node_config_t *nc);
list<node_config_t *> get_list(cluster_config_t *cc);
void elect_coordinator(cluster_config_t *cc);
node_config_t *cc_get_daemon(cluster_config_t *cc);
int load_cluster(int id, char *configi, cluster_config_t *cc);
#endif
