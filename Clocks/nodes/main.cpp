#include <stdlib.h>
#include <string.h>
#include <iomanip>
#include <string>
#include <iostream>

#include "protocol.h"
#include "multicast.h"
#include "cluster.h"
#include "util.h"
#include "logger.h"
#include "state.h"
#include "locks.h"

using namespace std;

void access_file(node_status_t *ns)
{
	int ff;
	int fd = ns->ns_lock_req->dlr_fd;
	read(fd, (void *)&ff, sizeof(int));
	cout << "Read value from file:" << ff <<endl;
	ff++;
	write(fd, (void*)&ff, sizeof(int));
}

void multicast_final_print(node_status_t *ns)
{
	causal_t *ct = ns->ns_causal;
	while(ct->c_count != 300)
		usleep(1000000);

	for(int i = 1; i <= ct->c_v_size; i++)
		cout << " " << ct->c_V[i] << endl;
	cout << endl;
}

void print_cluster_config(cluster_config_t *cc)
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

int main(int argc, char *argv[])
{
	int           rc;
	int           id;
	int           isdaemon;
	char         *nodelist;
	unsigned long iclock;
	char         *fname;

	/* Some parameters for floating point and log */
    	ios_base::sync_with_stdio(false); 
	setiosflags(ios::fixed);
	setprecision(15);

	if(argc != 6) {
		cout << "usage ./node <id> <nodelist> <d> <clock>" << endl;
		cout << "\
			 1. id -> Node identifier\n \
			 2. nodelist -> A file containing a list of all processes\n \
			 3. d -> Whether a node is time-daemon\n \
			 4. clock -> initial clock\n \
			 5. filename -> Accesses to this file are serialized\n";
			return -EINVAL;
	}

	id = atoi(argv[1]);
	nodelist = argv[2];
	isdaemon = atoi(argv[3]);
	iclock = atol(argv[4]);
	fname = argv[5];

	cout << "Node " << id << "is started with clock:" << iclock << endl;	
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

	cout << "On node:" << self->nc_id << endl;
	print_cluster_config(cc);
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
		bmt->b_diffs = new long [ll.size()+1];
		if(bmt->b_diffs == NULL) {
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

	if(ns->ns_isdmon)
		dl_init_lock(ns);

	rc = dl_init_request(ns, fname);
	if(rc != 0) {
		cr_log << "Distributed lock bootstrap failed..";
		return rc;
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
#if 0
	srand(time(NULL));
	for(int i = 0; i < 100; i++) {
		multicast(ns);
		usleep(1000+(rand()%1000));
	}

	cout << "------------------------------------------------" << endl;
	multicast_final_print(ns);
	cout << endl;

#endif

	int i = 0;
	while(i < 10) {
		dl_lock(ns);
		access_file(ns);
		dl_unlock(ns);
	}

	/* We will be back here when state machine reaches OFF state */
	delete ns;
	delete self;
	delete cc;
	return 0;
}
