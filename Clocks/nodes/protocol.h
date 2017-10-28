#ifndef __PROTO_H_
#define __PROTO_H_
#include "state.h"
enum msg_type {
	HELLO,
	SEND_CLK,
	SEND_CLK_REP,
	UPDATE_CLK,
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

typedef struct MESSAGE {
	unsigned long M_seq_no;
        enum msg_type M_type;
	union {
		hello_t M_u_h;
		sync_reply_t M_u_srt;
		update_clk_t M_u_uct;
	} u;
} msg_t;
bool process_msg(c_sock *cs, node_status_t *ns, msg_t *msg);
void send_hello_message(int id, node_status_t *ns);
void send_sync_message(node_status_t *ns, int id);
void send_time(node_status_t *ns, int dmon);
void send_update_time(node_status_t *ns, int id, double adjust);
#endif
