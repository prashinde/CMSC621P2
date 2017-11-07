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

enum conn_stat {
	NOT_CONNECTED,
	CONNECTED,
	READY_MULTICAST,
};

enum msg_ordering {
	CAUSAL,
	NOT_CAUSAL,
};

/*
 * Bootstrap cluster param
 * */
typedef struct cluster_bootstrap_param {
	int                cbp_id;
	char              *cbp_nodelist;
	bool               cbp_isdaemon;
	unsigned long      cbp_iclock;
	enum msg_ordering  cbp_causality;
	char              *cbp_fname;
} cluster_boot_t;

/* Node configuration */
typedef struct node_config {
	int            nc_id;
	enum conn_stat nc_status;
	int            nc_port_num;
	string         nc_ip_addr;
	c_sock        *nc_sock;
	unsigned long  nc_clock;
} node_config_t;

/* Global cluster config */
typedef struct cluster_config {
	int t_daemon;
	list<node_config_t *> cc_cluster;
} cluster_config_t;

/* Node status */
enum node_states {
	OFF,
	ON,
	CLK_SYNC_READY,
	CLK_SYNC_START,
	CLK_SYN_UPDATE,
	ENTERING_MULTICAST,
	ACCEPT_MULTICAST,
	READY_CAUSAL_MULTICAST
};

/* Berkley clock sync metadata */
typedef struct berkley {
	mutex          b_mx;
	long          *b_diffs;
	unsigned int   b_procs;
} berkley_t;

/* Buffered message */
typedef struct buffered_multicast {
	int bm_id;
	bool bm_dl;
	unsigned long *bm_V;
	unsigned long  bm_o_V[10];
} buffer_m_t;

/* Message to be delivered to application */
typedef struct app_msg {
	int           app_from;
	int           app_v_size;
	bool          from_buffer;
	bool          violates;
	unsigned long app_V[10];
	unsigned long current_V[10];
	unsigned long old_V[10];
} app_msg_t;

/* Causal ordering metadata */
typedef struct causal {
	long               c_count;
	unsigned long      c_v_size;
	unsigned long      c_V[10];
	list<buffer_m_t *> c_buffer;
	mutex              c_mx;
	condition_variable c_cv;
	queue<app_msg_t *> c_appq;
} causal_t;

/* Lock status */
enum lock_state {
	INIT,
	LOCKED,
	UNLOCKED,
};

/* Server side metadata about lock */
typedef struct d_lock {
	enum lock_state dl_state;
	queue<int>      dl_requests;
	int             dl_owner;
	mutex           dl_mx;
} d_lock_t;

enum request_status {
	GRANTED,
	REQUESTED,
	RELEASED,
};

/* Requestor side lock metadata */
typedef struct d_lock_requestor {
	enum request_status dlr_state;
	condition_variable  dlr_cv;
	mutex               dlr_mx;
} dlr_request_t;

/*
 * This structure is populated by each node
 * and keeps track of node progress.
 */
typedef struct node_status {
	bool              ns_isdmon;
	enum node_states  ns_state;
	node_config_t    *ns_self;
	c_sock            ns_server;
	cluster_config_t *ns_cc;
	berkley_t        *ns_berk;
	causal_t         *ns_causal;
	d_lock_t         *ns_lock;
	dlr_request_t    *ns_lock_req;
} node_status_t;

typedef struct node_con_ctx {
	node_status_t *ncc_ns;
	c_sock        *ncc_cs;
	node_config_t *ncc_connector;
} node_con_ctx_t;

node_status_t *bootstrap_cluster(cluster_boot_t *cbt);
node_config_t *cc_get_record(int id, cluster_config_t *cc);
int insert_node_config(cluster_config_t *cc, node_config_t *nc);
list<node_config_t *> get_list(cluster_config_t *cc);
void elect_coordinator(cluster_config_t *cc);
node_config_t *cc_get_daemon(cluster_config_t *cc);
int load_cluster(int id, char *config, cluster_config_t *cc);

void stop_cluster(node_status_t *ns);
cluster_config_t *cc_get_cc(node_status_t *ns);
int cc_nr_nodes(cluster_config_t *cc);

void BERKELY_SYNC(node_status_t *ns);
void berkley_clk_sync_rep(node_status_t *ns, int id, long cl_diff);
void berkley_adjust_clock(node_status_t *ns, double adjust);
#endif
