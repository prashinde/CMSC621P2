#include "protocol.h"
#include "multicast.h"

/* Set the process into the READ_MUKTICAST STATE.
 * Now a process can send a multicast message.
 **/
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
	ct->c_count = 0;
	ns->ns_causal = ct;
}

void multicast(node_status_t *ns, enum msg_ordering causality)
{
	causal_t *ct = ns->ns_causal;
	unique_lock<mutex> lck(ct->c_mx);
	ct->c_V[ns->ns_self->nc_id]++;
	send_mult_msg(ns, causality);
	lck.unlock();
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

/* 
 * IS message FIFO
 */
bool is_fifo(unsigned long local[10], unsigned long msg[10], int id, int size)
{
	return (local[id] + 1) == msg[id];
}

/*
 * IS message causal
 */
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

/*
 * Read sing message from multicast_recv_queue 
 */
app_msg_t *multicast_app_recv(node_status_t *ns)
{
	causal_t *ct = ns->ns_causal;
	app_msg_t *apt = new app_msg_t;
	unique_lock<mutex> lck(ct->c_mx);
	if(ct->c_appq.empty())
		ct->c_cv.wait(lck);
	apt = ct->c_appq.front();
	ct->c_appq.pop();
	lck.unlock();
	return apt;
}

/*
 * Put a message in multicast recv queue
 */
void deliver_message_to_app(causal_t *ct, unsigned long msg[10], int from, unsigned long o_V[10], bool from_buf, bool causal)
{
	int vsize = ct->c_v_size;
	app_msg_t *apt = new app_msg_t;

	for(int i = 1; i <= vsize; i++) {
		apt->app_V[i] = msg[i];
		apt->current_V[i] = ct->c_V[i];
		apt->old_V[i] = o_V[i];
	}
	apt->app_from  = from;
	apt->app_v_size = vsize;
	apt->from_buffer = from_buf;
	apt->violates = causal;
	ct->c_appq.push(apt);
}

/*
 * Check the buffer and deliver messages in causally ordered manner
 */
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
			if(ct->c_V[id] < (*it)->bm_V[id])
				ct->c_V[id] = (*it)->bm_V[id];
			//print_v((*it)->bm_V, ct->c_V, ct->c_v_size);
			deliver_message_to_app(ct, (*it)->bm_V, id, (*it)->bm_o_V, true, false);
			delete (*it)->bm_V;
			(*it)->bm_dl = true;
			it = (ct->c_buffer).erase(it);
			if((ct->c_buffer).size() == 0)
				break;
			it = (ct->c_buffer).begin();
		} else
			it++;
	}
}

/*
 * Multicast message is recived
 */
void mulicast_recv(node_status_t *ns, unsigned long msg[10], int id, enum msg_ordering order)
{
	causal_t *ct = ns->ns_causal;
	bool deliver = true;
	unsigned long o_V[10];
//	cout << "Locking the mcast mutex" << endl;

	
	unique_lock<mutex> lck(ct->c_mx);
	ct->c_count++;

	deliver = is_fifo(ct->c_V, msg, id, ct->c_v_size) && 
		  is_causal(ct->c_V, ns->ns_self->nc_id, msg, id, ct->c_v_size);


	if(order == NOT_CAUSAL) {
		//print_v(msg, ct->c_V, ct->c_v_size);
		for(int i = 1; i <= ct->c_v_size; i++)
			o_V[i] = ct->c_V[i];
		ct->c_V[id] = msg[id];
		/* We will tag a message which violates the causality. */
		deliver_message_to_app(ct, msg, id, o_V, false, !deliver);
		ct->c_cv.notify_all();
		lck.unlock();
		return ;
	}

	if(deliver) {
		//cr_log << "Deliver a message: On Node:" << ns->ns_self->nc_id << "from " << id << endl;
		for(int i = 1; i <= ct->c_v_size; i++)
			o_V[i] = ct->c_V[i];

		ct->c_V[id] = msg[id];
		deliver_message_to_app(ct, msg, id, o_V, false, false);
		//print_v(msg, ct->c_V, ct->c_v_size);
		deliver_buffered_messages(ns);
		ct->c_cv.notify_all();
	} else {
		buffer_m_t *mcast = new buffer_m_t;
		mcast->bm_id = id;
		mcast->bm_V = new unsigned long[(ct->c_v_size)+1];

		for(int i = 1; i <= ct->c_v_size; i++) {
			mcast->bm_V[i] = msg[i];
			mcast->bm_o_V[i] = ct->c_V[i];
		}
		mcast->bm_dl = false;
		(ct->c_buffer).push_back(mcast);
	}
	lck.unlock();
//	cout << "Unlocking the mcast mutex" << endl;
}
