#ifndef __PROTO_H_
#define __PROTO_H_

#include "locks.h"
#include "multicast.h"
#include "state.h"
enum msg_type {
	HELLO = 1,
	SEND_CLK,
	SEND_CLK_REP,
	UPDATE_CLK,
	MULTICAST_RD,
	MULTICAST,
	LOCK_REQUEST,
	LOCK_RELEASE,
	LOCK_GRANTED,
};

typedef struct hello {
	int           h_id;
} hello_t;

typedef struct sync_reply {
	int           h_id;
	long          diff;
} sync_reply_t;

typedef struct update_clk {
	double adjust;
} update_clk_t;

typedef struct mult_ready {
	int           h_id;
} mult_ready_t;

typedef struct clock_sync {
	unsigned long clock;
} clock_sync_t;

typedef struct mult {
	int           m_id;
	/* We will solve 10 limitation later */
	unsigned long vec[10];
} mult_t;

typedef struct d_lock_msg {
	int dl_id;
} lock_msg_t;

typedef struct MESSAGE {
	unsigned long M_seq_no;
        enum msg_type M_type;
	union {
		hello_t M_u_h;
		clock_sync_t M_u_cst;
		sync_reply_t M_u_srt;
		update_clk_t M_u_uct;
		mult_ready_t M_u_mrt;
		mult_t M_u_mult;
		lock_msg_t M_u_lmt;
	} u;
} msg_t;

bool process_msg(c_sock *cs, node_status_t *ns, msg_t *msg);
void send_hello_message(int id, node_status_t *ns);
void send_clock_message(node_status_t *ns, int id);
void send_time_difference(node_status_t *ns, int dmon, unsigned long clock);
void send_update_time(node_status_t *ns, int id, double adjust);
void clock_sync_recieved(node_status_t *ns, clock_sync_t srt);
void send_mult_ready(node_status_t *ns);
void send_mult_msg(node_status_t *ns);
void send_lock_request(node_status_t *ns);
void send_unlock_request(node_status_t *ns);
void send_lock_granted(node_status_t *ns, int id);
#endif
