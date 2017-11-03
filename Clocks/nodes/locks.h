#ifndef __LOCKS__H_
#define __LOCKS__H_

#include "cluster.h"
#include "protocol.h"

int dl_init_request(node_status_t *ns);
int dl_lock(node_status_t *ns);
int dl_unlock(node_status_t *ns);

int dl_init_lock(node_status_t *ns);
int dl_lock_req(node_status_t *ns, int id);
int dl_unlock_req(node_status_t *ns, int id);
int dl_lock_granted(node_status_t *ns);
#endif
