#ifndef __SOCK_H_
#define __SOCK_H_

/* Socket */
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/ip.h> 
#include <arpa/inet.h>
#include <unistd.h>

#include "logger.h"

class c_sock {
	struct sockaddr_in server_addr;
	struct sockaddr_in cli_addr;
	socklen_t addrlen;
	int sock;
	int cl_sock;
public:
	int get_cl_sock() { return this->cl_sock; };
	int c_sock_addr(char *ip, int port);
	int c_sock_connect();
	int c_sock_bind();
	int c_sock_listen();
	int c_sock_accept();
	ssize_t c_sock_read(void *buffer, size_t len);
	ssize_t c_sock_write(void *buffer, size_t len);
	ssize_t c_sock_read(int fd, void *buffer, size_t len);
	ssize_t c_sock_write(int fd, void *buffer, size_t len);
	void c_sock_close();
};
#endif
