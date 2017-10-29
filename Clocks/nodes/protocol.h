#ifndef __PROTO_H_
#define __PROTO_H_

#include "multicast.h"
#include "state.h"
enum msg_type {
	HELLO,
	SEND_CLK,
	SEND_CLK_REP,
	UPDATE_CLK,
	MULTICAST_RD,
	MULTICAST,
};

typedef struct hello {
	int           h_id;
} hello_t;

typedef struct sync_reply {
	int           h_id;
	unsigned long clock;
} sync_reply_t;

typedef struct update_clk {
	double adjust;
} update_clk_t;

typedef struct mult_ready {
	int           h_id;
} mult_ready_t;

typedef struct mult {
	int           m_id;
	/* We will solve 100 limitation later */
	unsigned long vec[100];
} mult_t;

typedef struct MESSAGE {
	unsigned long M_seq_no;
        enum msg_type M_type;
	union {
		hello_t M_u_h;
		sync_reply_t M_u_srt;
		update_clk_t M_u_uct;
		mult_ready_t M_u_mrt;
		mult_t M_u_mult;
	} u;
} msg_t;
bool process_msg(c_sock *cs, node_status_t *ns, msg_t *msg);
void send_hello_message(int id, node_status_t *ns);
void send_sync_message(node_status_t *ns, int id);
void send_time(node_status_t *ns, int dmon);
void send_update_time(node_status_t *ns, int id, double adjust);

void send_mult_ready(node_status_t *ns);
void send_mult_msg(node_status_t *ns);
#endif
