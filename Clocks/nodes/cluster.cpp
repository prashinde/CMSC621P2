#include "cluster.h"
#include "state.h"
#include "multicast.h"
#include "protocol.h"
#include "util.h"
#include "logger.h"
#include "locks.h"

static void print_cluster_config(cluster_config_t *cc)
{
	list<node_config_t *> ll = get_list(cc);
	
	list<node_config_t*>::iterator it;
	for(it = ll.begin(); it != ll.end(); ++it) {
		cr_log << "id:" << (*it)->nc_id << "\n";
		cr_log << "ip_addr:" << (*it)->nc_ip_addr << "\n";
		cr_log << "port num:" << (*it)->nc_port_num << "\n";
		cr_log << "connection status:" << (*it)->nc_status << "\n";
		cr_log << "-------------------------------------------\n";
		cr_log << "\n";
	}

	cout << endl;
}

node_status_t *bootstrap_cluster(cluster_boot_t *cbt)
{
	int           rc;
	int           id;
	int           isdaemon;
	char         *nodelist;
	unsigned long iclock;
	enum msg_ordering causality;

	id = cbt->cbp_id;
	isdaemon = cbt->cbp_isdaemon;
	nodelist = cbt->cbp_nodelist;
	iclock = cbt->cbp_iclock;

	cout << "Node " << id << "is started with clock:" << iclock << endl;
	cluster_config_t *cc = new cluster_config_t;
	if(cc == NULL) {
		return NULL;
	}

	/* Read and load the cluster configuration */
	rc = load_cluster(id, nodelist, cc);
	if(rc != 0) {
		cr_log << "Invalid cluster config:\n";
		return NULL;
	}

	node_config_t *self = cc_get_record(id, cc);
	if(self == NULL) {
		cr_log << "Invalid cluster config";
		delete cc;
		return NULL;
	}

	cout << "On node:" << self->nc_id << endl;
	print_cluster_config(cc);
	node_status_t *ns = new node_status_t;
	if(ns == NULL) {
		delete self;
		delete cc;
		return NULL;
	}

	if(isdaemon == 1) {
		berkley_t *bmt = new berkley_t;
		if(bmt == NULL) {
			cr_log << "Out of memory" << endl;
			delete cc;
			return NULL;
		}

		list<node_config_t *> ll = get_list(cc);
		bmt->b_procs = ll.size();
		bmt->b_diffs = new long [ll.size()+1];
		if(bmt->b_diffs == NULL) {
			delete cc;
			return NULL;
		}
		ns->ns_berk = bmt;
	}

	self->nc_clock = iclock;
	ns->ns_isdmon = ((isdaemon == 1) ? true : false); 
	ns->ns_state = OFF;
	ns->ns_self = self;
	ns->ns_cc = cc;

	if(ns->ns_isdmon)
		dl_init_lock(ns);

	rc = dl_init_request(ns);
	if(rc != 0) {
		cr_log << "Distributed lock bootstrap failed..";
		return NULL;
	}

	/* Start node's state machine. */
	START_STATE_MC(ns);

	/* ONLY Time daemon kiks the protocol */
	if(ns->ns_isdmon) {
		//cr_log << "Kick started the protocol.." << endl;
		BERKELY_SYNC(ns);
	}

	while(ns->ns_state != ENTERING_MULTICAST)
		usleep(1000);
	cout << "Entered Multicast state.." << endl;	
	print_cluster_config(cc);
	send_mult_ready(ns);

	/* ALL OTHER SHOULD WAIT FOR PROT TO COMPLETE */
	//WAIT_MULT_READY(ns);
	/*****************************************/
	cr_log << "***ID:" << self->nc_id << " Logical Clock:" << self->nc_clock << endl;
	/*****************************************/
	multicast_init_vector(ns);
	return ns;
}

int insert_node_config(cluster_config_t *cc, node_config_t *nc)
{
	(cc->cc_cluster).push_back(nc);
}

list<node_config_t  *> get_list(cluster_config_t *cc)
{
	return cc->cc_cluster;
}

node_config_t *cc_get_record(int id, cluster_config_t *cc)
{
	list<node_config_t *> ll = get_list(cc);	
	list<node_config_t*>::iterator it;
	for(it = ll.begin(); it != ll.end(); ++it) {
		if((*it)->nc_id == id)
			return *it;
	}
	return NULL;
}

void elect_coordinator(cluster_config_t *cc)
{
	int max = INT_MIN;
	list<node_config_t *> ll = get_list(cc);	
	list<node_config_t*>::iterator it;
	for(it = ll.begin(); it != ll.end(); ++it) {
		if((*it)->nc_id > max)
			max = (*it)->nc_id;
	}
	cc->t_daemon = max;
}

node_config_t *cc_get_daemon(cluster_config_t *cc)
{
	list<node_config_t *> ll = get_list(cc);	
	list<node_config_t*>::iterator it;
	for(it = ll.begin(); it != ll.end(); ++it) {
		if((*it)->nc_id == cc->t_daemon)
			return *it;
	}
	return NULL;
}
