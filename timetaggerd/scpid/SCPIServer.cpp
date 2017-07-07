/*
 * SCPIServer.cpp
 *
 *  Created on: 15.03.2013
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
#include <stdarg.h>

#include "SCPI.h"

#include "../../backend/Logger.h"


static void error(const char *fmt, ...) {
	va_list ap;
	va_start(ap, fmt);
	Logger::vlog( Logger::APPLICATION, Logger::LOG_ERROR, fmt, ap);
}

static void info(const char *fmt, ...) {
	va_list ap;
	va_start(ap, fmt);
	Logger::vlog( Logger::APPLICATION, Logger::LOG_INFO, fmt, ap);
}


SCPIServer::SCPIServer(SCPI *_scpi, int _port)
: scpi(_scpi), port(_port), running(false), listener(0) {

	socket_list_len=16;
	socket_list=(SCPISocket **)calloc(sizeof(SCPISocket *), socket_list_len);
}

void SCPIServer::start() {
	info("starting scpi server on port %d..", port);
	listener=open_socket();
	if (listener<=0) {
		error("starting scpi server failed.");
	} else {
		thread=new current_thread(SCPIServer::run, this);
	}
}

void SCPIServer::stop() {
	running=false;
}


int SCPIServer::open_socket() {
	int fd;
    int rc;
    int on = 1;
    struct sockaddr_in servaddr;

    /* Configure TCP Server */
    bzero(&servaddr, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr=htonl(INADDR_ANY);
    servaddr.sin_port=htons(this->port);

    /* Create socket */
    fd = socket(AF_INET,SOCK_STREAM, 0);
    if (fd < 0) {
    	error("error starting server, socket() failed (%s).", strerror(errno));
        return (-1);
    }

    /* Set address reuse enable */
    rc = setsockopt(fd, SOL_SOCKET,  SO_REUSEADDR, (char *)&on, sizeof(on));
    if (rc < 0) {
    	error("error starting server, setsockopt() failed (%s).", strerror(errno));
        close(fd);
        return (-1);
    }

    /* Set non blocking */
    rc = ioctl(fd, FIONBIO, (char *)&on);
    if (rc < 0) {
    	error("error starting server, ioctl() failed (%s).", strerror(errno));
        close(fd);
        return (-1);
    }

    /* Bind to socket */
    rc = bind(fd, (struct sockaddr *)&servaddr, sizeof(servaddr));
    if (rc < 0) {
    	error("error starting server (%s). port in use?", strerror(errno));
        close(fd);
        return (-1);
    }

    /* Listen on socket */
    listen(fd, 1);
    if (rc < 0) {
    	error("error starting server, listen() failed (%s).", strerror(errno));
        close(fd);
        return (-1);
    }

    return fd;
}


void SCPIServer::run(void *arg) {	// threadproc
	SCPIServer *server = (SCPIServer *)  arg;

	//current_sleep(200);	// give start() some time to continue

	info("server up, waiting for connection.");
	server->scpi->register_server(server);

	server->running=true;
    while (server->running) {
		struct sockaddr_in cliaddr;
		socklen_t clilen;

		clilen = sizeof(cliaddr);
		int clifd = accept(server->listener, (struct sockaddr *)&cliaddr, &clilen);

		if (clifd < 0) continue;
		info("Connection established %s\r\n", inet_ntoa(cliaddr.sin_addr));

		SCPISocket *client=new SCPISocket(server, clifd, cliaddr);
		client->start();
    }

    //TODO: server->stop_all_connections();
    close(server->listener);

	server->scpi->unregister_server(server);
	info("tcpip on port server terminated..", server->port);
}


SCPI *SCPIServer::get_scpi() {
	return scpi;
}

bool SCPIServer::register_socket(SCPISocket *sock) {
	socket_list_mutex.lock();
	size_t n;
	for (n=0; n<socket_list_len; ++n) {
		if (socket_list[n]==0) {
			socket_list[n]=sock;
			socket_list_mutex.unlock();
			return true;
		}
	}
	size_t len=socket_list_len;
	len+=len/2;
	SCPISocket **list=(SCPISocket **) realloc(socket_list, sizeof(SCPISocket **) * len);
	if (!list) {
		socket_list_mutex.unlock();
		return false;
	}
	for (n=0; n<len; ++n) {
		if (n==socket_list_len) {
			list[n]=sock;
		} else if (n<socket_list_len) {
			list[n]=socket_list[n];
		} else {
			list[n]=0;
		}
	}
	socket_list=list;
	socket_list_len=len;
	socket_list_mutex.unlock();
	return true;
}

bool SCPIServer::unregister_socket(SCPISocket *sock) {
	socket_list_mutex.lock();
	size_t n;
	for (n=0; n<socket_list_len; ++n) {
		if (socket_list[n]==sock) {
			socket_list[n]=0;
			socket_list_mutex.unlock();
			return true;
		}
	}
	socket_list_mutex.unlock();
	return false;
}

void SCPIServer::terminate() {
	// terminate all sockets
	size_t n;
	for (n=0; n<socket_list_len; ++n) {
		if (socket_list[n])
			socket_list[n]->stop();
	}
	for (n=0; n<socket_list_len; ++n) {
		if (socket_list[n])
			socket_list[n]->terminate();
	}
	// stop server thread
	if (running)
		stop();
	if (thread)
		thread->join();
	thread=0;
}
