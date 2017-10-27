#include <stdlib.h>
#include <string.h>

#include <string>
#include <iostream>

#include "cluster.h"
#include "util.h"
#include "logger.h"
using namespace std;


/*
 * Parse a space seperate line.
 */
int parse_line(stringstream &ss, char *fi, cluster_config_t *cc)
{
	string s;
	int i = 0;
	node_config_t *nc = new node_config_t;
	if(nc == NULL)
		return -ENOMEM;
	/* Parse a single line */
	while(getline(ss, s, ' ')) {
		i++;
		switch(i) {
			case 1:
			if(is_string_num(s)) {
				cr_log << "cluster config corrupted" << endl;
				return -EINVAL;
			}
			nc->nc_id = stol(s, NULL);
			break;

			case 2:
			nc->nc_ip_addr = s;
			break;

			case 3:
			if(is_string_num(s)) {
				cr_log << "cluster config corrupted" << endl;
				return -EINVAL;
			}
			nc->nc_port_num = stol(s, NULL);
			break;

			default:
			cr_log << "cluster config corrupted" << endl;
			return -EINVAL;
			break;
		}
	}

	insert_node_config(cc, nc);
	return 0;
}

/*
 * Parse a file line by line.
 * Each line is a cluster participant.
 */
int load_cluster(int id, char *config, cluster_config_t *cc)
{
	int      rc;
	ifstream fs;
	string   line;

	fs.open(config);
	if(fs.fail()) {
		rc = errno;
		cr_log << "Unable to open file:" << config << ":" << rc <<endl; 
		return rc;
	}
	
	/* Read line */
	while(getline(fs, line)) {
		if(fs.bad()) {
			rc = errno;
			fs.close();
			cr_log << "Error reading from a file: " << errno << endl;
			return rc;
		}
		stringstream ss(line);
		parse_line(ss, config, cc);
	}
	
	if(fs.eof()) {
		/* Do nothing */
	}

	fs.close();
	elect_coordinator(cc);
	return 0;
}
