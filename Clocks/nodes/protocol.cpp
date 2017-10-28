#include "protocol.h"

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
		default:
		cr_log << "Impossible";
		break;
	}
	return rc;
}
