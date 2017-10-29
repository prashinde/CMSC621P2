#include "protocol.h"
#include "multicast.h"

void multicast_ready(node_status_t *ns, int id)
{
	cluster_config_t *cc = ns->ns_cc;
	node_config_t *connected_nc;
	
	connected_nc = cc_get_record(id, cc);
	if(connected_nc == NULL) {
		cr_log << " Invalid configuration close connection.";
		return ;
	}

	//cr_log << "Recieved hellow from " << msg.h_id << "to " << ns->ns_self->nc_id << endl;
	connected_nc->nc_status = READY_MULTICAST;
}

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

static void print_v(unsigned long msg[100], unsigned long self[100], int size)
{
	cr_log << "[";
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

void mulicast_recv(node_status_t *ns, unsigned long msg[100], int id)
{
	causal_t *ct = ns->ns_causal;
	bool deliver = true;
	unique_lock<mutex> lck(ct->c_mx);
	for(int i = 1; i <= ct->c_v_size; i++) {
		if(i == id)
			continue;
		if(msg[i] > ct->c_V[i]) {
			deliver = false;
			break;
		}
	}

	if(deliver) {
		cr_log << "Deliver a message: On Node:" << ns->ns_self->nc_id << "from " << id << endl;
		ct->c_V[id] = msg[id];
		print_v(msg, ct->c_V, ct->c_v_size);
	} else {
		cr_log << "Buffer a message: On Node:" << ns->ns_self->nc_id << "from " << id << endl;
		print_v(msg, ct->c_V, ct->c_v_size);
	}
	lck.unlock();
}
