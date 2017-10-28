#include "state.h"
#include "protocol.h"

/*
 * I am sure a time deamon
 * */
void BERKELY_SYNC(node_status_t *ns)
{
	cluster_config_t *cc = ns->ns_cc;
	list<node_config_t *> ll = get_list(cc);
	list<node_config_t*>::iterator it;

	ns->ns_state = CLK_SYNC_START;
	for(it = ll.begin(); it != ll.end(); ++it) {
		if((*it)->nc_id == ns->ns_self->nc_id)
			continue;
		send_sync_message(ns, (*it)->nc_id);
	}
}

void clock_sync_recieved(node_status_t *ns)
{
	cluster_config_t *cc = ns->ns_cc;
	send_time(ns, cc->t_daemon);
}

static void run_computations(node_status_t *ns)
{
	berkley_t *bmt = ns->ns_berk;
	list<node_config_t *> ll = get_list(ns->ns_cc);
	unsigned long sum = 0;
	double avg_t;

	cr_log << " About to run comps...:" << ll.size() << endl;
	for(int i = 1; i <= ll.size(); i++) {
		sum += bmt->b_times[i];
	}
	sum += ns->ns_self->nc_clock;
	avg_t = sum/ll.size();
	
	for(int i = 1; i <= ll.size(); i++) {
		send_update_time(ns, i, avg_t-(bmt->b_times[i]));
	}

}

void berkley_clk_sync_rep(node_status_t *ns, int id, unsigned long clock)
{
	berkley_t *bmt = ns->ns_berk;
	bmt->b_times[id] = clock;
	unique_lock<mutex> lck(bmt->b_mx);
	bmt->b_procs--;
	/* 1 because daemon does not get a response from itself. */
	if(bmt->b_procs == 1) {
		lck.unlock();
		cr_log << " Going to run comps..." << endl;
		/* send messages */
		run_computations(ns);
		return ;
	}
	cr_log << " Processes:" << bmt->b_procs << endl;
	lck.unlock();
}
