#include "sock.h"

int c_sock :: c_sock_addr(char *ip, int port)
{
	int rc;

	this->sock = socket(AF_INET, SOCK_STREAM, 0);
	if(this->sock == -1) {
		cr_log << "Unable to create a socket.:" << errno << endl;
		return errno;
	}

	this->server_addr.sin_family = AF_INET;
	
	rc = inet_aton(ip, &(this->server_addr.sin_addr));
	if(rc == 0) {
		cr_log << "Server ip address invalid: " << ip << endl;
		return -EINVAL;
	}

	int optval = 1;
	setsockopt(this->sock, SOL_SOCKET, SO_REUSEPORT, &optval, sizeof(optval));

	this->server_addr.sin_port = htons(port);
	return 0;
}

int c_sock::c_sock_connect()
{
	int rc;

	rc = connect(this->sock, (struct sockaddr *)&(this->server_addr), sizeof(this->server_addr));
	if (rc < 0) {
		cr_log << "Server not connected." << endl;
		return rc;
	}

	return 0;
}

int c_sock :: c_sock_bind()
{
	int rc;

	rc = bind(this->sock, (struct sockaddr *)&(this->server_addr), sizeof(this->server_addr));
	if(rc < 0) {
		cr_log << "Bind failed.:" << errno << endl;
		return rc;
	}
	return 0;
}

int c_sock :: c_sock_listen()
{
	listen(this->sock, 128);
}

int c_sock :: c_sock_accept()
{
	this->addrlen = sizeof(this->cli_addr);
	this->cl_sock = accept(this->sock, (struct sockaddr *)&this->cli_addr, &this->addrlen);
	if(this->cl_sock < 0) {
		cr_log << "Unable to accept the connection:" << errno << endl;
		return errno;
	}

	return 0;
}

ssize_t c_sock::c_sock_read(void *buffer, size_t len)
{
	return recv(this->sock, buffer, len, 0);
}

ssize_t c_sock::c_sock_write(void *buffer, size_t len)
{
	return send(this->sock, buffer, len, 0);
}

ssize_t c_sock::c_sock_read(int fd, void *buffer, size_t len)
{
	return recv(fd, buffer, len, 0);
}

ssize_t c_sock::c_sock_write(int fd, void *buffer, size_t len)
{
	return send(fd, buffer, len, 0);
}

void c_sock::c_sock_close()
{
	close(this->sock);
}
