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
	
	pthread_mutex_init(&ct->c_mx, NULL);
	ct->c_v_size = ll.size();
	ct->c_count = 0;
	ns->ns_causal = ct;
}

void multicast(node_status_t *ns)
{
	causal_t *ct = ns->ns_causal;
	pthread_mutex_lock(&ct->c_mx);
	ct->c_V[ns->ns_self->nc_id]++;
	send_mult_msg(ns);
	pthread_mutex_unlock(&ct->c_mx);
}

static void print_v(unsigned long msg[10], unsigned long self[10], int size)
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

bool is_fifo(unsigned long local[10], unsigned long msg[10], int id, int size)
{
	return (local[id] + 1) == msg[id];
}

bool is_causal(unsigned long local[10], int self, unsigned long msg[10], int id, int size)
{
	bool deliver = true;
	for(int i = 1; i <= size; i++) {
		if(i == id || i == self)
			continue;

		if(msg[i] > local[i]) {
			deliver = false;
			break;
		}
	}
	return deliver;
}

void deliver_buffered_messages(node_status_t *ns)
{
	causal_t *ct = ns->ns_causal;
	list<buffer_m_t *>::iterator it;
	bool deliver = false;

	if((ct->c_buffer).size() == 0)
		return ;

	for(it = (ct->c_buffer).begin(); it != (ct->c_buffer).end(); ) {
		deliver = is_fifo(ct->c_V, (*it)->bm_V, (*it)->bm_id, ct->c_v_size) && 
			  is_causal(ct->c_V, ns->ns_self->nc_id, (*it)->bm_V, (*it)->bm_id, ct->c_v_size);
		if(deliver) {
			int id = (*it)->bm_id;
			cout << "D:";
			if(ct->c_V[id] < (*it)->bm_V[id])
				ct->c_V[id] = (*it)->bm_V[id];
			print_v((*it)->bm_V, ct->c_V, ct->c_v_size);
			delete (*it)->bm_V;
			(*it)->bm_dl = true;
			it = (ct->c_buffer).erase(it);		
		} else
			it++;
	}
}

void mulicast_recv(node_status_t *ns, unsigned long msg[10], int id)
{
	causal_t *ct = ns->ns_causal;
	bool deliver = true;
//	cout << "Locking the mcast mutex" << endl;
	pthread_mutex_lock(&ct->c_mx);
	ct->c_count++;

	deliver = is_fifo(ct->c_V, msg, id, ct->c_v_size) && 
		  is_causal(ct->c_V, ns->ns_self->nc_id, msg, id, ct->c_v_size);

	if(deliver) {
		//cr_log << "Deliver a message: On Node:" << ns->ns_self->nc_id << "from " << id << endl;
		cr_log << "D:";
		ct->c_V[id] = msg[id];
		print_v(msg, ct->c_V, ct->c_v_size);
		deliver_buffered_messages(ns);
	} else {
		//cr_log << "Buffer a message: from Node: "<< id << endl;
		cr_log << "B:";
		print_v(msg, ct->c_V, ct->c_v_size);

		buffer_m_t *mcast = new buffer_m_t;
		mcast->bm_id = id;
		mcast->bm_V = new unsigned long[(ct->c_v_size)+1];

		for(int i = 1; i <= ct->c_v_size; i++)
			mcast->bm_V[i] = msg[i];
		mcast->bm_dl = false;
		(ct->c_buffer).push_back(mcast);
	}
	pthread_mutex_unlock(&ct->c_mx);
//	cout << "Unlocking the mcast mutex" << endl;
}
