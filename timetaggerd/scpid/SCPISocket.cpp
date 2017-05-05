/*
 * SCPISocket.cpp
 *
 *  Created on: 19.03.2013
 *      Author: mag
 */

#include <string.h>
#include <stdlib.h>
#include <unistd.h>

#include <errno.h>
#include <netinet/in.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <arpa/inet.h>


#include "SCPI.h"
#include "../../backend/Logger.h"


static void info(const char *fmt, ...) {
	va_list ap;
	va_start(ap, fmt);
	Logger::vlog( Logger::APPLICATION, Logger::LOG_INFO, fmt, ap);
}


SCPISocket::SCPISocket(SCPIServer *_server, int _fd, struct sockaddr_in addr)
: server(_server),
  fd(_fd), remote_addr(addr),
  running(0),
  thread(0) {

}

SCPISocket::~SCPISocket() {
	//TODO: free
}

void SCPISocket::start() {
	thread=new current_thread(SCPISocket::run, this);
}

void SCPISocket::stop() {
	running=true;
}

void SCPISocket::terminate() {
	if (running)
		stop();
	if (thread)
		thread->join();
	thread=0;
}


void SCPISocket::run(void *arg) {

	SCPISocket *socket = (SCPISocket *) arg;
	SCPI *scpi = socket->server->get_scpi();

	info("new connection from %s",inet_ntoa(socket->remote_addr.sin_addr));

	socket->server->register_socket(socket);

	SCPIContext *scpi_context=scpi->new_context();
	scpi_context->set_socket(socket->fd, socket->remote_addr);

	char *buffer=(char *)calloc(1, DEFAULT_BUFFERSIZE);
	int timeoutcounter=0;
	socket->running=true;
	while (socket->running) {
		// read chunk, wait max 100msec
		int ret=scpi_context->do_read(buffer, DEFAULT_BUFFERSIZE, 100);

		if (ret==1 && buffer[0]==4) {		// ^D => EOF
			socket->running=false;

		} else if (ret==0) {
			timeoutcounter+=100;
			if  (timeoutcounter > DEFAULT_TIMEOUT) {
				scpi_context->execute(buffer,0);
				timeoutcounter=0;
			}

		} else if (ret<0) {
			socket->running=false;

		} else {
			scpi_context->execute(buffer,ret);
			timeoutcounter=0;
		}
	}

	delete scpi_context;

	socket->server->unregister_socket(socket);

	close(socket->fd);
	info("connection closed.");

}
