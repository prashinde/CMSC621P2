#include <stdlib.h>
#include <string.h>
#include <iomanip>
#include <string>
#include <iostream>

#include "multicast.h"
#include "cluster.h"
#include "util.h"
#include "logger.h"
#include "state.h"

using namespace std;

void print_cluster_config(cluster_config_t *cc)
{
	list<node_config_t *> ll = get_list(cc);
	
	list<node_config_t*>::iterator it;
	for(it = ll.begin(); it != ll.end(); ++it) {
		cout << "id:" << (*it)->nc_id << "\n";
		cout << "ip_addr:" << (*it)->nc_ip_addr << "\n";
		cout << "port num:" << (*it)->nc_port_num << "\n";
		cout << "-------------------------------------------\n";
		cout << "\n";
	}
}

int main(int argc, char *argv[])
{
	int           rc;
	int           id;
	int           isdaemon;
	char         *nodelist;
	unsigned long iclock;

	/* Some parameters for floating point and log */
    	ios_base::sync_with_stdio(false); 
	setiosflags(ios::fixed);
	setprecision(15);

	if(argc != 5) {
		cout << "usage ./node <id> <nodelist> <d> <clock>" << endl;
		cout << "\
			 1. id -> Node identifier\n \
			 2. nodelist -> A file containing a list of all processes\n \
			 3. d -> Whether a node is time-daemon\n \
			 4. clock -> initial clock";
			return -EINVAL;
	}

	id = atoi(argv[1]);
	nodelist = argv[2];
	isdaemon = atoi(argv[3]);
	iclock = atol(argv[4]);
	
	cluster_config_t *cc = new cluster_config_t;
	if(cc == NULL) {
		return -ENOMEM;
	}

	/* Read and load the cluster configuration */
	rc = load_cluster(id, nodelist, cc);
	if(rc != 0) {
		cr_log << "Invalid cluster config:\n";
		return rc;
	}

	node_config_t *self = cc_get_record(id, cc);
	if(self == NULL) {
		cr_log << "Invalid cluster config";
		delete cc;
		return -EINVAL;
	}

	
	node_status_t *ns = new node_status_t;
	if(ns == NULL) {
		delete self;
		delete cc;
		return -ENOMEM;
	}

	if(isdaemon == 1) {
		berkley_t *bmt = new berkley_t;
		if(bmt == NULL) {
			cr_log << "Out of memory" << endl;
			delete cc;
			return -ENOMEM;
		}

		list<node_config_t *> ll = get_list(cc);
		bmt->b_procs = ll.size();
		bmt->b_times = new unsigned long [ll.size()+1];
		if(bmt->b_times == NULL) {
			delete cc;
			return -ENOMEM;
		}
		ns->ns_berk = bmt;
	}

	self->nc_clock = iclock;
	ns->ns_isdmon = ((isdaemon == 1) ? true : false); 
	ns->ns_state = OFF;
	ns->ns_self = self;
	ns->ns_cc = cc;

	/* Start node's state machine. */
	START_STATE_MC(ns);

	/* ONLY Time daemon kiks the protocol */
	if(ns->ns_isdmon) {
		cr_log << "Kick started the protocol.." << endl;
		BERKELY_SYNC(ns);
	}
	/* ALL OTHER SHOULD WAIT FOR PROT TO COMPLETE */
	while(ns->ns_state != MULT)
		;
	/*****************************************/
	cr_log << "***ID:" << self->nc_id << " Logical Clock:" << self->nc_clock << endl;
	/*****************************************/

	multicast_init_vector(ns);
	multicast(ns);

	/* We will be back here when state machine reaches OFF state */
	delete ns;
	delete self;
	delete cc;
	return 0;
}
