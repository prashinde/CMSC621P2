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
public:
	int c_sock_addr(string ip, int port);
	int c_sock_connect();
	int c_sock_bind();
	int c_sock_listen();
	c_sock* c_sock_accept();
	ssize_t c_sock_read(void *buffer, size_t len);
	ssize_t c_sock_write(void *buffer, size_t len);
	void c_sock_close();
};
#endif
