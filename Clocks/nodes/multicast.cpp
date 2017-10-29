#include "protocol.h"
#include "multicast.h"

void multicast_init_vector(node_status_t *ns)
{
	causal_t *ct = new causal_t;
	if(ct == NULL) {
		cr_log << "Out of memory" << endl;
		return ;
	}

	list<node_config_t *> ll = get_list(ns->ns_cc);
	for(int i = 1; i <= ll.size(); i++)
		ct->c_V[i] = ns->ns_self->nc_clock;

	ct->c_v_size = ll.size();
	ns->ns_causal = ct;
}

void multicast(node_status_t *ns)
{
	causal_t *ct = ns->ns_causal;
	unique_lock<mutex> lck(ct->c_mx);
	ct->c_V[ns->ns_self->nc_id]++;
	send_mult_msg(ns);
	lck.unlock();
}
