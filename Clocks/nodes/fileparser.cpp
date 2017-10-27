#include <stdlib.h>
#include <string.h>

#include <string>
#include <iostream>

#include "node.h"
#include "util.h"
#include "logger.h"
using namespace std;


/*
 * Parse a space seperate line.
 */
int parse_line(stringstream &ss, char *f)
{
	string s;
	int i = 0;

	/* Parse a single line */
	while(getline(ss, s, ' ')) {
		i++;
		switch(i) {
			case 1:
			if(is_string_num(s)) {
				cr_log << "cluster config corrupted" << endl;
				return -EINVAL;
			}

			cout << "id:" << stol(s, NULL) << "\n";
			break;

			case 2:

			cout << "ip:" << s << "\n";
			break;

			case 3:
			if(is_string_num(s)) {
				cr_log << "cluster config corrupted" << endl;
				return -EINVAL;
			}
			cout << "port no:" << stol(s, NULL) << "\n";
			break;

			default:
			cr_log << "cluster config corrupted" << endl;
			return -EINVAL;
			break;
		}
	}

	return 0;
}

/*
 * Parse a file line by line.
 * Each line is a cluster participant.
 */
int load_cluster(int id, char *config)
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
		parse_line(ss, config);
	}
	
	if(fs.eof()) {
		/* Do nothing */
	}

	fs.close();
	return 0;
}
