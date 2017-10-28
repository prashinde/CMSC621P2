#include "protocol.h"

void send_update_time(node_status_t *ns, int id, double adjust)
{
	cluster_config_t *cc = ns->ns_cc;
	node_config_t *node;

	msg_t *msg = new msg_t;
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
	berkley_clk_sync_rep(ns, msg.h_id, msg.clock);
}

void send_time(node_status_t *ns, int dmon)
{
	cluster_config_t *cc = ns->ns_cc;
	node_config_t *node;

	msg_t *msg = new msg_t;
	msg->M_type = SEND_CLK_REP;

	sync_reply_t srt;
	srt.h_id = ns->ns_self->nc_id;
	srt.clock = ns->ns_self->nc_clock;

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
	clock_sync_recieved(ns);
}

void send_sync_message(node_status_t *ns, int id)
{
	cluster_config_t *cc = ns->ns_cc;
	node_config_t *client;

	msg_t *msg = new msg_t;
	msg->M_type = SEND_CLK;

	client = cc_get_record(id, cc);
	if(client == NULL) {
		cr_log << " Invalid configuration close connection.";
		return ;
	}

	if(client->nc_status != CONNECTED) {
		cr_log << "Client not connected. Abort";
	}

	c_sock *c_wr = client->nc_sock;
	ssize_t ret = c_wr->c_sock_write((void *)msg, sizeof(msg_t));
	if(ret < 0) {
		cr_log << "Error in writing into the socket:" << ret << " Errno:"<< errno << endl;
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
	msg->M_type = HELLO;
	msg->u.M_u_h = h;

	boss = cc_get_record(id, cc);
	if(boss == NULL) {
		cr_log << " Invalid configuration close connection.";
		return ;
	}

	if(boss->nc_status != CONNECTED) {
		cr_log << "BOSS not connected.";
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
	
	connected_nc = cc_get_record(msg.h_id, cc);
	if(connected_nc == NULL) {
		cr_log << " Invalid configuration close connection.";
		return -EINVAL;
	}

	cr_log << "Recieved hellow from " << msg.h_id << "to " << ns->ns_self->nc_id << endl;
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
		default:
		cr_log << "Impossible:" << msg->M_type << endl;
		break;
	}
	return rc;
}