#ifndef __PROTO_H_
#define __PROTO_H_
#include "state.h"
enum msg_type {
	HELLO,
	SYNC,
	SYNC_REP,
	UPDATE,
	MULTICAST,
};

typedef struct hello {
	int           h_id;
} hello_t;

typedef struct MESSAGE {
	unsigned long M_seq_no;
        enum msg_type M_type;
	union {
		hello_t *M_u_h;
	} u;
} msg_t;
bool process_msg(c_sock *cs, node_status_t *ns, msg_t *msg);
void send_hello_message(int id, node_status_t *ns);
#endif
