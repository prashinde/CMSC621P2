#include <stdlib.h>
#include <string.h>
#include <iomanip>
#include <string>
#include <iostream>

#include "cluster.h"
#include "util.h"
#include "logger.h"

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

	cout << "-------------------------------------------\n";
	cout << "Everyone elected:" << (cc_get_daemon(cc))->nc_id << "\n";
	cout << "-------------------------------------------\n";
	delete self;
	delete cc;
	return 0;
}
