#include <stdlib.h>
#include <string.h>
#include <iomanip>
#include <string>
#include <iostream>

#include "node.h"
#include "util.h"
#include "logger.h"

using namespace std;

int main(int argc, char *argv[])
{
	int   rc;
	int   id;
	int   isdaemon;
	char *nodelist;

	/* Some parameters for floating point and log */
    	ios_base::sync_with_stdio(false); 
	setiosflags(ios::fixed);
	setprecision(15);

	if(argc != 4) {
		cout << "usage ./node <id> <nodelist> <d>" << endl;
		cout << "\
			 1. id -> Node identifier\n \
			 2. nodelist -> A file containing a list of all processes\n \
			 3. d -> Whether a node is time-daemon\n";
			return -EINVAL;
	}

	id = atoi(argv[1]);
	nodelist = argv[2];
	isdaemon = atoi(argv[3]);
	/* Read and load the cluster configuration */
	rc = load_cluster(id, nodelist);
	if(rc != 0) {
		cr_log << "Invalid cluster config:\n";
		return rc;
	}

	return 0;
}
