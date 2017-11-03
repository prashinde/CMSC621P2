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

typedef struct sender {
	node_status_t     *ns;
	enum msg_ordering  causal;
	int                nr_msg;
} sender_t;

typedef struct reciever {
	node_status_t *ns;
	int            nr_msg;
} recv_t;

static void access_file(FILE *fp)
{
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

static void print_v(unsigned long self[10], unsigned long msg[10], int size)
{
	cout << "[";
	for(int i = 1; i <= size; i++) {
		cout << self[i] << " ";
	}
	cout << "] " ;

	cout << "[";
	for(int i = 1; i <= size; i++) {
		cout << msg[i] << " ";
	}
	cout << "] " << endl;
}

void *sfunc(void *ctx)
{
	sender_t *sctx = (sender_t*)ctx;
	srand(time(NULL));
	for(int i = 0; i < sctx->nr_msg; i++) {
		multicast(sctx->ns, sctx->causal);
		usleep(1000+(rand()%1000));
	}
}

void *rfunc(void *ctx)
{
	recv_t *rctx = (recv_t*)ctx;
	for(int i = 0; i < rctx->nr_msg; i++) {
		app_msg_t *msg = multicast_app_recv(rctx->ns);
		if(msg == NULL) {
			cr_log << "Recieved NULL message from communication!" << endl;
		} else { 
			print_v(msg->current_V, msg->app_V, msg->app_v_size);
			delete msg;
		}
	}
}

int main(int argc, char *argv[])
{
	FILE *fp;
	enum msg_ordering causality;
	/* Some parameters for floating point and log */
    	ios_base::sync_with_stdio(false); 
	setiosflags(ios::fixed);
	setprecision(15);

	if(argc != 8) {
		cout << "usage ./node <id> <nodelist> <d> <clock>" << endl;
		cout << "\
			 1. id -> Node identifier\n \
			 2. nodelist -> A file containing a list of all processes\n \
			 3. d -> Whether a node is time-daemon\n \
			 4. clock -> initial clock\n \
			 5. causality -> 0/1 Messages to be ordered?\n \
			 6. number of msg -> number of multicasts \n \
			 7. filename -> Accesses to this file are serialized\n";
			return -EINVAL;
	}

	cluster_boot_t *cbt = new cluster_boot_t;

	cbt->cbp_id = atoi(argv[1]);
	cbt->cbp_nodelist = argv[2];
	cbt->cbp_isdaemon = atoi(argv[3]);
	cbt->cbp_iclock = atol(argv[4]);
	cbt->cbp_causality = (atoi(argv[5]) == 1 ? CAUSAL : NOT_CAUSAL);

	causality = cbt->cbp_causality;
	node_status_t *ns = bootstrap_cluster(cbt);
	if(ns == NULL) {
		delete cbt;
		return -EINVAL;
	}

	sender_t *sctx = new sender_t;
	sctx->ns = ns;
	sctx->causal = causality;
	sctx->nr_msg = atoi(argv[6]);
	
	recv_t *rctx = new recv_t;
	rctx->ns = ns;
	rctx->nr_msg = atoi(argv[6]);

	thread s_thread(sfunc, sctx);
	thread r_thread(rfunc, rctx);

	s_thread.join();
	r_thread.join();
	
	cout << "------------------------------------------------" << endl;
	multicast_final_print(ns);
	cout << endl;

	fp = fopen(argv[7], "r+");
	if(fp == NULL) {
		cr_log << "Unable to open file." << errno << endl;
		return -EINVAL;
	}

	setvbuf(fp, NULL, _IONBF, 0);

	int i = 0;
	while(i < 10) {
		dl_lock(ns);
		access_file(fp);
		dl_unlock(ns);
		usleep(1000+(rand()%1000));
		i++;
	}

	fclose(fp);

	while(1)
		usleep(1000000);
	/* We will be back here when state machine reaches OFF state */
	delete ns;
	delete rctx;
	delete sctx;
	return 0;
}
