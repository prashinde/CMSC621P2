#include <fcntl.h>

#include "locks.h"
#include "logger.h"

int dl_init_request(node_status_t *ns)
{
	dlr_request_t *lock_req;

	lock_req = new dlr_request_t;
	if(lock_req == NULL) {
		return -ENOMEM;
	}

	lock_req->dlr_state = RELEASED;
	ns->ns_lock_req = lock_req;
	return 0;
}

int dl_init_lock(node_status_t *ns)
{
	d_lock_t *lock;

	lock = new d_lock_t;
	if(lock == NULL) {
		return -ENOMEM;
	}

	lock->dl_state = UNLOCKED;
	lock->dl_owner = -1;

	ns->ns_lock = lock;
	return 0;
}

int dl_lock_req(node_status_t *ns, int id)
{
	d_lock_t *lock = ns->ns_lock;
	unique_lock<mutex> lck(lock->dl_mx);
	if(lock->dl_state == UNLOCKED) {
		lock->dl_state = LOCKED;
		lock->dl_owner = id;
		cr_log << "Lock granted:" << id << endl;
		lck.unlock();
		send_lock_granted(ns, id);
	} else {
		cr_log << "Requst queued:" << id << endl;
		lock->dl_requests.push(id);	
		lck.unlock();
	}
	return 0;
}

int dl_unlock_req(node_status_t *ns, int id)
{
	d_lock_t *lock = ns->ns_lock;
	unique_lock<mutex> lck(lock->dl_mx);
	if(lock->dl_state == UNLOCKED) {
		/* IMPOSSIBLE */
	}

	cr_log << "Unlock the lock from:" << id << endl;
	lock->dl_state = UNLOCKED;
	lock->dl_owner = -1;
	/* If queue is empty, just release the lock. */
	if(!lock->dl_requests.empty()) {
		int nowner = lock->dl_requests.front();
		lock->dl_requests.pop();
		lock->dl_state = LOCKED;
		lock->dl_owner = nowner;
		cr_log << "New lock granted owner:" << nowner << endl;
		lck.unlock();
		send_lock_granted(ns, nowner);
	} else {
		lck.unlock();
	}

	return 0;
}

int dl_lock_granted(node_status_t *ns)
{
	dlr_request_t *lr = ns->ns_lock_req;
	unique_lock<mutex> lck(lr->dlr_mx);
	lr->dlr_state = GRANTED;
	cr_log << "Recived lock Grant:" << ns->ns_self->nc_id << endl;
	lr->dlr_cv.notify_all();
	lck.unlock();
}

int dl_lock(node_status_t *ns)
{
	dlr_request_t *lr = ns->ns_lock_req;
	
	unique_lock<mutex> lck(lr->dlr_mx);
	lr->dlr_state = REQUESTED;

	cr_log << "Sending lock request:" << ns->ns_self->nc_id << endl;
	send_lock_request(ns);

	lr->dlr_cv.wait(lck);
	lck.unlock();
	return 0;
}

int dl_unlock(node_status_t *ns)
{
	cr_log << "Sending unlock request:" << ns->ns_self->nc_id << endl;
	send_unlock_request(ns);
	return 0;
}
