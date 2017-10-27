#ifndef __LOGGER_H_
#define __LOGGER_H_

#include <iostream>
using namespace std;
#define cr_log cout << __FILE__<< ":" << __FUNCTION__ << ": " << __LINE__ <<": ";cout.flush()

/* Reference : https://cppcodetips.wordpress.com/2014/01/02/a-simple-logger-class-in-c */

class Logger {

};

#endif
