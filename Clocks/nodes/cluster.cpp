#include "cluster.h"
using namespace std;

int insert(cluster_config_t *cc, node_config_t *nc)
{
	(cc->cc_cluster).push_back(nc);
}

list<node_config_t  *> get_list(cluster_config_t *cc)
{
	return cc->cc_cluster;
}
