#include "cluster.h"
using namespace std;

int insert_node_config(cluster_config_t *cc, node_config_t *nc)
{
	(cc->cc_cluster).push_back(nc);
}

list<node_config_t  *> get_list(cluster_config_t *cc)
{
	return cc->cc_cluster;
}

node_config_t *cc_get_record(int id, cluster_config_t *cc)
{
	list<node_config_t *> ll = get_list(cc);	
	list<node_config_t*>::iterator it;
	for(it = ll.begin(); it != ll.end(); ++it) {
		if((*it)->nc_id == id)
			return *it;
	}
	return NULL;
}

void elect_coordinator(cluster_config_t *cc)
{
	int max = INT_MIN;
	list<node_config_t *> ll = get_list(cc);	
	list<node_config_t*>::iterator it;
	for(it = ll.begin(); it != ll.end(); ++it) {
		if((*it)->nc_id > max)
			max = (*it)->nc_id;
	}
	cc->t_daemon = max;
}

node_config_t *cc_get_daemon(cluster_config_t *cc)
{
	list<node_config_t *> ll = get_list(cc);	
	list<node_config_t*>::iterator it;
	for(it = ll.begin(); it != ll.end(); ++it) {
		if((*it)->nc_id == cc->t_daemon)
			return *it;
	}
	return NULL;
}
