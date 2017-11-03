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

void deliver_buffered_messages(node_status_t *ns)
{
	causal_t *ct = ns->ns_causal;
	list<buffer_m_t *>::iterator it;
	bool deliver = false;

	if((ct->c_buffer).size() == 0)
		return ;

	for(it = (ct->c_buffer).begin(); it != (ct->c_buffer).end(); ) {
		deliver = true;
		for(int i = 1; i <= ct->c_v_size; i++) {
			if(i == (*it)->bm_id)
				continue;
			if((*it)->bm_dl)
				continue;
			if((*it)->bm_V[i] > ct->c_V[i]) {
				deliver = false;
				break;
			}
		}

		if(deliver) {
			cout << "Buffer Message delivered:";
			print_v((*it)->bm_V, ct->c_V, ct->c_v_size);
			(*it)->bm_dl = true;
			//it = ll.erase(it);		
		}
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
	for(int i = 1; i <= ct->c_v_size; i++) {
		if(i == id)
			continue;
		if(msg[i] > ct->c_V[i]) {
			deliver = false;
			break;
		}
	}
	//cout << "From: " << id << " Multicast count: " << ct->c_count << endl;

	if(deliver) {
		cr_log << "Deliver a message: On Node:" << ns->ns_self->nc_id << "from " << id << endl;
		ct->c_V[id] = msg[id];
		print_v(msg, ct->c_V, ct->c_v_size);
		//deliver_buffered_messages(ns);
	} else {
		cr_log << "Buffer a message:" << endl;
		//print_v(msg, ct->c_V, ct->c_v_size);

		/*buffer_m_t *mcast = new buffer_m_t;
		mcast->bm_id = id;
		mcast->bm_V = new unsigned long[(ct->c_v_size)+1];

		for(int i = 1; i <= ct->c_v_size; i++)
			mcast->bm_V[i] = msg[i];
		mcast->bm_dl = false;
		(ct->c_buffer).push_back(mcast);*/
	}
	pthread_mutex_unlock(&ct->c_mx);
//	cout << "Unlocking the mcast mutex" << endl;
}
