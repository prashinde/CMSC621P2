#include "protocol.h"

void process_lock_request(c_sock *cs, node_status_t *ns, lock_msg_t lmt)
{
	dl_lock_req(ns, lmt.dl_id);
}

void process_lock_release(c_sock *cs, node_status_t *ns, lock_msg_t lmt)
{
	dl_unlock_req(ns, lmt.dl_id);
}

void process_lock_granted(c_sock *cs, node_status_t *ns)
{
	dl_lock_granted(ns);
}

void send_lock_granted(node_status_t *ns, int id)
{
	cluster_config_t *cc = ns->ns_cc;
	node_config_t *node;

	msg_t *msg = new msg_t;
	msg->M_seq_no = 0;
	msg->M_type = LOCK_GRANTED;

	node = cc_get_record(id, cc);
	if(node == NULL) {
		cr_log << " Invalid configuration close connection.";
		return ;
	}

	if(node->nc_status == NOT_CONNECTED) {
		cr_log << "Client server not connected not connected. Abort";
		return ;
	}

	c_sock *c_wr = node->nc_sock;
	ssize_t ret = c_wr->c_sock_write((void *)msg, sizeof(msg_t));
	if(ret < 0) {
		cr_log << "Error in writing into the socket:" << ret << " Errno:"<< errno << endl;
	}
}

void send_unlock_request(node_status_t *ns)
{
	cluster_config_t *cc = ns->ns_cc;
	node_config_t *node;

	msg_t *msg = new msg_t;
	msg->M_seq_no = 0;
	msg->M_type = LOCK_RELEASE;

	lock_msg_t lmt;
	lmt.dl_id = ns->ns_self->nc_id;

	msg->u.M_u_lmt = lmt;	
	node = cc_get_record(cc->t_daemon, cc);
	if(node == NULL) {
		cr_log << " Invalid configuration close connection.";
		return ;
	}

	if(node->nc_status == NOT_CONNECTED) {
		cr_log << "Central server not connected not connected. Abort";
		return ;
	}

	c_sock *c_wr = node->nc_sock;
	ssize_t ret = c_wr->c_sock_write((void *)msg, sizeof(msg_t));
	if(ret < 0) {
		cr_log << "Error in writing into the socket:" << ret << " Errno:"<< errno << endl;
	}
}

void send_lock_request(node_status_t *ns)
{
	cluster_config_t *cc = ns->ns_cc;
	node_config_t *node;

	msg_t *msg = new msg_t;
	msg->M_seq_no = 0;
	msg->M_type = LOCK_REQUEST;

	lock_msg_t lmt;
	lmt.dl_id = ns->ns_self->nc_id;

	msg->u.M_u_lmt = lmt;	
	node = cc_get_record(cc->t_daemon, cc);
	if(node == NULL) {
		cr_log << " Invalid configuration close connection.";
		return ;
	}

	if(node->nc_status == NOT_CONNECTED) {
		cr_log << "Central server not connected not connected. Abort";
		return ;
	}

	c_sock *c_wr = node->nc_sock;
	ssize_t ret = c_wr->c_sock_write((void *)msg, sizeof(msg_t));
	if(ret < 0) {
		cr_log << "Error in writing into the socket:" << ret << " Errno:"<< errno << endl;
	}
}

void process_multicast_message(c_sock *cs, node_status_t *ns, mult_t msg)
{
	usleep(1000+(rand()%1000));
	mulicast_recv(ns, msg.vec, msg.m_id, msg.m_order);
}

void send_mult_msg(node_status_t *ns, enum msg_ordering causality)
{
	cluster_config_t *cc = ns->ns_cc;
	node_config_t *node;
	list<node_config_t *> ll = get_list(cc);	

	msg_t *msg = new msg_t;
	msg->M_seq_no = 0;
	msg->M_type = MULTICAST;

	mult_t mul;

	for(int i = 1; i <= ns->ns_causal->c_v_size; i++)
		mul.vec[i] = ns->ns_causal->c_V[i];

	mul.m_id    = ns->ns_self->nc_id;
	mul.m_order = causality; 
	msg->u.M_u_mult = mul;
	list<node_config_t*>::iterator it;
	for(it = ll.begin(); it != ll.end(); ++it) {
		/* Do not multicast to yourself */
		if((*it)->nc_id == ns->ns_self->nc_id)
			continue;
		//cout << " Sending to process:" <<(*it)->nc_id<< endl;
		while((*it)->nc_status != READY_MULTICAST)
			usleep(1000);
		c_sock *c_wr = (*it)->nc_sock;
		ssize_t ret = c_wr->c_sock_write((void *)msg, sizeof(msg_t));
		if(ret < 0) {
			cr_log << "Error in writing into the socket:" << ret << " Errno:"<< errno << endl;
		}
	}
}

static void process_multicast_rd(c_sock *cs, node_status_t *ns, mult_ready_t mrt)
{
	multicast_ready(ns, mrt.h_id);
}

void send_mult_ready(node_status_t *ns)
{
	cluster_config_t *cc = ns->ns_cc;
	node_config_t *node;
	list<node_config_t *> ll = get_list(cc);	

	msg_t *msg = new msg_t;
	msg->M_seq_no = 0;
	msg->M_type = MULTICAST_RD;

	mult_ready_t mrt;
	mrt.h_id = ns->ns_self->nc_id;

	msg->u.M_u_mrt = mrt;

	list<node_config_t*>::iterator it;
	for(it = ll.begin(); it != ll.end(); ++it) {
		if((*it)->nc_status == NOT_CONNECTED) {
			cout << "Node "<< (*it)->nc_id << " Not connected.." << endl;
			return;
		}
		c_sock *c_wr = (*it)->nc_sock;
		ssize_t ret = c_wr->c_sock_write((void *)msg, sizeof(msg_t));
		if(ret < 0) {
			cr_log << "Error in writing into the socket:" << ret << " Errno:"<< errno << endl;
		}
	}
}

static void process_update_clk(c_sock *cs, node_status_t *ns, update_clk_t msg)
{
	berkley_adjust_clock(ns, msg.adjust);
}

void send_update_time(node_status_t *ns, int id, double adjust)
{
	cluster_config_t *cc = ns->ns_cc;
	node_config_t *node;

	msg_t *msg = new msg_t;
	msg->M_seq_no = 0;
	msg->M_type = UPDATE_CLK;

	update_clk_t uct;
	uct.adjust = adjust;

	msg->u.M_u_uct = uct;

	node = cc_get_record(id, cc);
	if(node == NULL) {
		cr_log << " Invalid configuration close connection.";
		return ;
	}

	if(node->nc_status != CONNECTED) {
		cr_log << "Client not connected. Abort";
		return ;
	}

	c_sock *c_wr = node->nc_sock;
	ssize_t ret = c_wr->c_sock_write((void *)msg, sizeof(msg_t));
	if(ret < 0) {
		cr_log << "Error in writing into the socket:" << ret << " Errno:"<< errno << endl;
	}
}

static void process_clk_sync_rep(c_sock *cs, node_status_t *ns, sync_reply_t msg)
{
	berkley_clk_sync_rep(ns, msg.h_id, msg.diff);
}

void send_time_difference(node_status_t *ns, int dmon, unsigned long dclock)
{
	cluster_config_t *cc = ns->ns_cc;
	node_config_t *node;

	msg_t *msg = new msg_t;
	msg->M_seq_no = 0;
	msg->M_type = SEND_CLK_REP;

	sync_reply_t srt;
	srt.h_id = ns->ns_self->nc_id;
	srt.diff = (ns->ns_self->nc_clock - dclock);

	cout << "node: " << srt.h_id << " Sending difference to tdaemon: " << srt.diff << endl;
	msg->u.M_u_srt = srt;	
	node = cc_get_record(dmon, cc);
	if(node == NULL) {
		cr_log << " Invalid configuration close connection.";
		return ;
	}

	if(node->nc_status != CONNECTED) {
		cr_log << "Client not connected. Abort";
	}

	c_sock *c_wr = node->nc_sock;
	ssize_t ret = c_wr->c_sock_write((void *)msg, sizeof(msg_t));
	if(ret < 0) {
		cr_log << "Error in writing into the socket:" << ret << " Errno:"<< errno << endl;
	}

}

static void process_clk_sync_start(c_sock *cs, node_status_t *ns, msg_t *msg)
{
	clock_sync_recieved(ns, msg->u.M_u_cst);
}

void send_clock_message(node_status_t *ns, int id)
{
	cluster_config_t *cc = ns->ns_cc;
	node_config_t *client;

	msg_t *msg = new msg_t;
	msg->M_seq_no = 0;
	msg->M_type = SEND_CLK;

	clock_sync_t cst;
	cst.clock = ns->ns_self->nc_clock;
	msg->u.M_u_cst = cst;

	client = cc_get_record(id, cc);
	if(client == NULL) {
		cr_log << " Invalid configuration close connection.";
		return ;
	}

	if(client->nc_status != CONNECTED) {
		cr_log << "Client not connected. Abort";
		return ;
	}

	c_sock *c_wr = client->nc_sock;
	ssize_t ret = c_wr->c_sock_write((void *)msg, sizeof(msg_t));
	if(ret < 0) {
		cr_log << "Error in writing into the socket:" << ret << " Errno:"<< errno << endl;
	}
}

void process_bye_message(node_status_t *ns, bye_t b_msg)
{
	cluster_config_t *cc = ns->ns_cc;
	node_config_t *node;

	node = cc_get_record(b_msg.b_id, cc);
	if(node == NULL) {
		return ;
	}

	node->nc_status = NOT_CONNECTED;
}

void send_bye_message(node_status_t *ns)
{
	cluster_config_t *cc = ns->ns_cc;
	list<node_config_t *> ll = get_list(cc);	
	msg_t *msg = new msg_t;
	/* HAndle filure */

	msg->M_seq_no = 0;
	msg->M_type = BYE;

	msg->u.M_u_b.b_id = ns->ns_self->nc_id;
	list<node_config_t*>::iterator it;
	for(it = ll.begin(); it != ll.end(); ++it) {
		if((*it)->nc_status == NOT_CONNECTED) {
			cout << "Node "<< (*it)->nc_id << " Not connected.." << endl;
			continue;
		}
		c_sock *c_wr = (*it)->nc_sock;
		ssize_t ret = c_wr->c_sock_write((void *)msg, sizeof(msg_t));
		if(ret < 0) {
			cr_log << "Error in writing into the socket:" << ret << " Errno:"<< errno << endl;
		}
		c_wr->c_sock_close();
		(*it)->nc_status = NOT_CONNECTED;
	}
}

void send_hello_message(int id, node_status_t *ns)
{
	cluster_config_t *cc = ns->ns_cc;
	node_config_t *boss;

	msg_t *msg = new msg_t;
	/* HAndle filure */

	hello_t h;
	/* Handle failure */

	h.h_id = ns->ns_self->nc_id;

	msg->M_seq_no = 0;
	msg->M_type = HELLO;
	msg->u.M_u_h = h;

	boss = cc_get_record(id, cc);
	if(boss == NULL) {
		cr_log << " Invalid configuration close connection.";
		delete msg;
		return ;
	}

	if(boss->nc_status != CONNECTED) {
		cr_log << "BOSS not connected.";
		delete msg;
		return ;
	}

	c_sock *c_wr = boss->nc_sock;

	ssize_t ret = c_wr->c_sock_write((void *)msg, sizeof(msg_t));
	if(ret < 0) {
		cr_log << "Error in writing into the socket:" << ret << " Errno:"<< errno << endl;
	}
}

static int process_hello_message(c_sock *cs, node_status_t *ns, hello_t msg)
{
	cluster_config_t *cc = ns->ns_cc;
	node_config_t *connected_nc;

	cout << "Recieved hellow message from: " << msg.h_id << endl;	
	connected_nc = cc_get_record(msg.h_id, cc);
	if(connected_nc == NULL) {
		cr_log << "Unable to find valid conf for node:"<< msg.h_id << endl;
		return -EINVAL;
	}

	connected_nc->nc_status = CONNECTED;
	connected_nc->nc_sock = cs;
	return 0;
}

bool process_msg(c_sock *cs, node_status_t *ns, msg_t *msg)
{
	bool rc = true;
	int pt;
	switch(msg->M_type) {
		case HELLO:
		pt = process_hello_message(cs, ns, msg->u.M_u_h);
		if(pt != 0) {
			rc = false;
		}
		break;

		case SEND_CLK:
		process_clk_sync_start(cs, ns, msg);
		break;
		case SEND_CLK_REP:
		process_clk_sync_rep(cs, ns, msg->u.M_u_srt);
		break;
		
		case UPDATE_CLK:
		process_update_clk(cs, ns, msg->u.M_u_uct);
		break;

		case MULTICAST_RD:
		process_multicast_rd(cs, ns, msg->u.M_u_mrt);
		break;

		case MULTICAST:
		/* TODO: Ignore messages during clock sync. */
		process_multicast_message(cs, ns, msg->u.M_u_mult);
		break;

		case LOCK_REQUEST:
		process_lock_request(cs, ns, msg->u.M_u_lmt);
		break;

		case LOCK_RELEASE:
		process_lock_release(cs, ns, msg->u.M_u_lmt);
		break;

		case LOCK_GRANTED:
		process_lock_granted(cs, ns);
		break;

		case BYE:
		process_bye_message(ns, msg->u.M_u_b);
		rc = false;
		break;

		default:
		/* Break the protocol and I will stop talkong to you! */
		rc = false;
		cr_log << "Impossible:" << msg->M_type << endl;
		break;
	}
	return rc;
}
