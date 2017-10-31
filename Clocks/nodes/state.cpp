#include <thread>

#include "protocol.h"
#include "state.h"

static void *incoming(void *ctx)
{
	/* Recieve */
	bool cont = true;
	node_con_ctx_t *ctxt = (node_con_ctx_t *)ctx;
	c_sock *cs = ctxt->ncc_cs;
	node_status_t *ns = ctxt->ncc_ns;
	ssize_t rcv = 0;

	while(cont) {
		msg_t *msg = new msg_t;
		rcv = 0;
		rcv = cs->c_sock_read(msg, sizeof(msg_t));
		cont = process_msg(cs, ns, msg);
	}
	cs->c_sock_close();
	return NULL;
}

static void listen_loop(c_sock *ss, node_status_t *ns)
{
	ns->ns_state = ON; 
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
		delete bs;
		return ;
	}

	int retry_t = 10;
	unsigned long step = 1000;
	unsigned long wi = 1000;
	while(1) {
		rc = bs->c_sock_connect();
		if(rc == 0)
			break;
		if(retry_t == 0) {
			cr_log << "Socket not connected:" << errno << endl;
			delete bs;
			return ;
		}
		wi += step;
		//cr_log << "Trying to connect.." << boss->nc_ip_addr << " " << boss->nc_port_num << endl;
		//cr_log << "Socket not connected... retrying to connect in :" << wi << "seconds" << " self:" << self->nc_id << endl;
		usleep(wi);
		retry_t--;
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

static void CONNECT_TO_SELF(node_status_t *ns)
{
	connect_to_one_boss(ns, ns->ns_self);
}

static void CONNECT_TO_BOSSES(node_status_t *ns)
{
	node_config_t *self = ns->ns_self;
	cluster_config_t *cc = ns->ns_cc;
	list<node_config_t *> ll = get_list(cc);
	
	list<node_config_t*>::iterator it;
	for(it = ll.begin(); it != ll.end(); ++it) {
		if(self->nc_id > (*it)->nc_id)
			continue;
		connect_to_one_boss(ns, *it);
	}

}

static void WAIT_MC(cluster_config_t *cc, int self)
{
	list<node_config_t *> ll = get_list(cc);
	list<node_config_t*>::iterator it;
	for(it = ll.begin(); it != ll.end(); ++it) {
		while((*it)->nc_status != CONNECTED)
			usleep(1000);
	}

}

void WAIT_MULT_READY(node_status_t *ns)
{
	list<node_config_t *> ll = get_list(ns->ns_cc);
	list<node_config_t*>::iterator it;
	for(it = ll.begin(); it != ll.end(); ++it) {
		while((*it)->nc_status != READY_MULTICAST)
			usleep(1000);
	}

	cout << "Node:" << ns->ns_self->nc_id << " enters READY_MULTICAST" << endl;
	ns->ns_state = ACCEPT_MULTICAST;
}

static void print_node_status(cluster_config_t *cc)
{
	list<node_config_t *> ll = get_list(cc);
	
	list<node_config_t*>::iterator it;
	for(it = ll.begin(); it != ll.end(); ++it) {
		cr_log << "id:" << (*it)->nc_id << "\n";
		cr_log << "ip_addr:" << (*it)->nc_ip_addr << "\n";
		cr_log << "port num:" << (*it)->nc_port_num << "\n";
		cr_log << "Connection:" << (*it)->nc_status << "\n";
		cr_log << "-------------------------------------------\n";
		cr_log << "\n";
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
		CONNECT_TO_SELF(ns);
		WAIT_MC(ns->ns_cc, ns->ns_self->nc_id);
	} else {
		CONNECT_TO_BOSSES(ns);
		WAIT_MC(ns->ns_cc, ns->ns_self->nc_id);
	}
	/* All connections are setup. We are ready for clock sync. */
	ns->ns_state = CLK_SYNC_READY;
	//cr_log << "****************STATE of Process " << ns->ns_self->nc_id << "is in sync.." << endl;
	//print_node_status(ns->ns_cc);
	server_thread.detach();
}
