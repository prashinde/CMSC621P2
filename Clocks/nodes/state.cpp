#include <thread>

#include "protocol.h"
#include "state.h"

static void *incoming(void *ctx)
{
	/* Send and Recieve */
	bool cont = true;
	node_con_ctx_t *ctxt = (node_con_ctx_t *)ctx;
	c_sock *cs = ctxt->ncc_cs;
	node_status_t *ns = ctxt->ncc_ns;

	while(cont) {
		msg_t *msg = new msg_t;
		cs->c_sock_read(msg, sizeof(msg_t));		
		cont = process_msg(cs, ns, msg);
	}
	return NULL;
}

static void listen_loop(c_sock *ss, node_status_t *ns)
{
	while(1) {
		c_sock *cs = ss->c_sock_accept();
		if(cs == NULL) {
			cr_log << "Socket not connected:" << errno << endl;
			return ;
		}

		/* Ohh someone wants to talk to me! :) */
	       	node_con_ctx_t *ctx = new node_con_ctx_t;
		if(ctx == NULL) {
			/* Handle, retry? */
		}
		ctx->ncc_cs = cs;
		ctx->ncc_ns = ns;

		/* Handle incoming traffic from this node */
		thread t1(incoming, ctx);
		t1.detach();
	}
}

static void *listener(void *ctx)
{
	int rc;
	node_status_t *ns = (node_status_t*)ctx;

	c_sock *ss = new c_sock;
	if(ss == NULL) {
		cr_log<<"Unable to open a socket:" << endl;
		return NULL;
	}

	rc = ss->c_sock_addr(ns->ns_self->nc_ip_addr, ns->ns_self->nc_port_num);
	if(rc != 0) {
		delete ss;
		cr_log << "Invalid Addresses" << endl;
		return NULL;
	}
	rc = ss->c_sock_bind();
	if(rc < 0) {
		delete ss;
		cr_log << "Unable to bind" << endl;
		return NULL;
	}
	ss->c_sock_listen();
	listen_loop(ss, ns);
	ns->ns_state = ON; 
}

static void WAIT_MC()
{
}

static void connect_to_one_boss(node_status_t *ns, node_config_t *boss)
{
	int rc;
	node_config_t *self = ns->ns_self;

	c_sock *bs = new c_sock;
	if(ns == NULL) {
		cr_log<<"Unable to open a socket:" << endl;
		return ;	
	}

	rc = bs->c_sock_addr(boss->nc_ip_addr, boss->nc_port_num);
	if(rc != 0) {
		cr_log << "Invalid Addresses" << endl;
		delete ns;
		return ;
	}

	rc = bs->c_sock_connect();
	if(rc != 0) {
		cr_log << "Socket not connected:" << errno << endl;
		delete ns;
		return ;
	}

	boss->nc_sock = bs;
	boss->nc_status = CONNECTED;
	node_con_ctx_t *ctx = new node_con_ctx_t;
	if(ctx == NULL) {
		/* Handle, retry? */
	}
	ctx->ncc_cs = bs;
	ctx->ncc_ns = ns;

	/* Handle incoming traffic from this node */
	thread t1(incoming, ctx);
	t1.detach();
	send_hello_message(boss->nc_id, ns);
}

static void CONNECT_TO_BOSSES(node_status_t *ns)
{
	node_config_t *self = ns->ns_self;
	cluster_config_t *cc = ns->ns_cc;
	list<node_config_t *> ll = get_list(cc);
	
	list<node_config_t*>::iterator it;
	for(it = ll.begin(); it != ll.end(); ++it) {
		if(self->nc_id >= (*it)->nc_id)
			continue;
		connect_to_one_boss(ns, *it);
		cout << "\n";
	}

}

void START_STATE_MC(node_status_t *ns)
{
	/* spawn a thread */
	thread server_thread(listener, ns);

	/* Wait for server to start */
	while(ns->ns_state == OFF)
		;

	/* Server started. If you are a time daemon wait for everyone else to connect.
	 * OR connect to every process that is bigger than you.
	 **/
	if(ns->ns_isdmon) {
		WAIT_MC();
	} else {
		CONNECT_TO_BOSSES(ns);
	}

	server_thread.join();
}
