#include <stdlib.h>
#include <string.h>
#include <iomanip>
#include <string>
#include <assert.h>
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
	FILE *fp = ns->ns_lock_req->dlr_fp;
	int ff;
	int ff_t;

	fseek(fp, 0L, SEEK_SET);
	fscanf(fp, "%d", &ff);
	ff++;
	fseek(fp, 0L, SEEK_SET);
	fprintf(fp, "%d", ff);
	fseek(fp, 0L, SEEK_SET);
	fscanf(fp, "%d", &ff_t);

	cr_log << "Writte Value:" << ff << " Read value:" << ff_t << endl;
	assert(ff == ff_t);
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

int main(int argc, char *argv[])
{
	enum msg_ordering causality;
	/* Some parameters for floating point and log */
    	ios_base::sync_with_stdio(false); 
	setiosflags(ios::fixed);
	setprecision(15);

	if(argc != 7) {
		cout << "usage ./node <id> <nodelist> <d> <clock>" << endl;
		cout << "\
			 1. id -> Node identifier\n \
			 2. nodelist -> A file containing a list of all processes\n \
			 3. d -> Whether a node is time-daemon\n \
			 4. clock -> initial clock\n \
			 5. causality -> 0/1 Messages to be ordered?\n \
			 6. filename -> Accesses to this file are serialized\n";
			return -EINVAL;
	}

	cluster_boot_t *cbt = new cluster_boot_t;

	cbt->cbp_id = atoi(argv[1]);
	cbt->cbp_nodelist = argv[2];
	cbt->cbp_isdaemon = atoi(argv[3]);
	cbt->cbp_iclock = atol(argv[4]);
	cbt->cbp_causality = (atoi(argv[5]) == 1 ? CAUSAL : NOT_CAUSAL);
	cbt->cbp_fname = argv[6];

	causality = cbt->cbp_causality;
	node_status_t *ns = bootstrap_cluster(cbt);
	if(ns == NULL) {
		delete cbt;
		return -EINVAL;
	}

	srand(time(NULL));
	for(int i = 0; i < 100; i++) {
		multicast(ns, causality);
		usleep(1000+(rand()%1000));
	}

	cout << "------------------------------------------------" << endl;
	multicast_final_print(ns);
	cout << endl;

	int i = 0;
	while(i < 10) {
		dl_lock(ns);
		access_file(ns);
		dl_unlock(ns);
		usleep(1000+(rand()%1000));
		i++;
	}

	while(1)
		usleep(100000);
	/* We will be back here when state machine reaches OFF state */
	delete ns;
	return 0;
}
