#include <errno.h>
#include "util.h"
#include "logger.h"

bool is_string_num(string s)
{
	return find_if_not(s.begin(), s.end(), ::isdigit) != s.end();
}
