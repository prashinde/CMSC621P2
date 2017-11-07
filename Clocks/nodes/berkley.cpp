#include "state.h"
#include "protocol.h"

/*
 * I am sure a time deamon
 * starts a sync protocol. Sends a 
 * clock to cliets.
 **/
void BERKELY_SYNC(node_status_t *ns)
{
	cluster_config_t *cc = ns->ns_cc;
	list<node_config_t *> ll = get_list(cc);
	list<node_config_t*>::iterator it;

	cout << "Starting Berkley Clock sync.." << endl;
	ns->ns_state = CLK_SYNC_START;
	for(it = ll.begin(); it != ll.end(); ++it) {
		/*if((*it)->nc_id == ns->ns_self->nc_id)
			continue;*/
		cout << "Sending clock sync message to node: " << (*it)->nc_id << endl;
		send_clock_message(ns, (*it)->nc_id);
	}
}

/*
 * Called by client nodes after recieving clock-sync from
 * server.
 */
void clock_sync_recieved(node_status_t *ns, clock_sync_t cst)
{
	cluster_config_t *cc = ns->ns_cc;
	cout << "Clock sync is recieved from daemon: " << cst.clock << endl;
	send_time_difference(ns, cc->t_daemon, cst.clock);
}

/*
 * Calculate the adjustment for every node.
 */
static void run_computations(node_status_t *ns)
{
	berkley_t *bmt = ns->ns_berk;
	list<node_config_t *> ll = get_list(ns->ns_cc);
	long sum = 0;
	long average;
	int num_elems = ll.size(); /* Thanks to c++ esoteric behavioir */

	for(int i = 1; i <= ll.size(); i++) {
		//cout << "Sum:" << sum << "b_diff[i]:" << bmt->b_diffs[i] << endl;
		sum += bmt->b_diffs[i];
	}

	average = sum/num_elems;
	//cout << "Average: " << average << "Sum: " << sum << "ll.size " << ll.size() << endl;	
	for(int i = 1; i <= ll.size(); i++) {
		//cout << "Sending " << average-(bmt->b_diffs[i]) << " to " << i << endl;
		send_update_time(ns, i, average-(bmt->b_diffs[i]));
	}
}

/*
 * Called by server, after it recieves a reply from client
 */
void berkley_clk_sync_rep(node_status_t *ns, int id, long cl_diff)
{
	berkley_t *bmt = ns->ns_berk;
	bmt->b_diffs[id] = cl_diff;
	cout << "Recieved clock difference from: " << id << " diff:" << cl_diff << endl;
	unique_lock<mutex> lck(bmt->b_mx);
	bmt->b_procs--;
	/* 1 because daemon does not get a response from itself. */
	if(bmt->b_procs == 0) {
		/* send messages */
		run_computations(ns);
		lck.unlock();
		return ;
	}
	lck.unlock();
}

/*
 * Called by client after it recieves an adjustment from the server.
 */
void berkley_adjust_clock(node_status_t *ns, double adjust)
{
	unsigned long oclock = ns->ns_self->nc_clock;
	ns->ns_self->nc_clock += adjust;
	ns->ns_state = ENTERING_MULTICAST;
	cout << "Old clock: " << oclock << " Recieved adjustment: " << adjust << " New clock: " << ns->ns_self->nc_clock << endl;
	//cr_log << "Sending multicast ready message:" << endl;
}
