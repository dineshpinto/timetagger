/*
 * SCPIBind.cpp
 *
 *  Created on: 22.03.2013
 *      Author: mag
 */


#include <unistd.h>
#include <strings.h>
#include <string.h>
#include <errno.h>
#include <netinet/in.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <arpa/inet.h>

#include "../../backend/Logger.h"

#define SCPI_BIND_SERVER_ONLY

#include "SCPIBind.h"

static void info(const char *fmt, ...) {
	va_list ap;
	va_start(ap, fmt);
	Logger::vlog( Logger::APPLICATION, Logger::LOG_INFO, fmt, ap);
}

static void debug(const char *fmt, ...) {
	va_list ap;
	va_start(ap, fmt);
	Logger::vlog( Logger::APPLICATION, Logger::LOG_DEBUG, fmt, ap);
}

static void error(const char *fmt, ...) {
	va_list ap;
	va_start(ap, fmt);
	Logger::vlog( Logger::APPLICATION, Logger::LOG_ERROR, fmt, ap);
}

SCPIBind::SCPIBind(const char *name, int port) {
	prepare(name);
}

SCPIBind::~SCPIBind() {
	stop();
}

void SCPIBind::start() {
	thread=new current_thread(SCPIBind::run, this);
}

void SCPIBind::prepare(const char *name) {
	if (name && *name) {
		int len=strlen(name);
		if (len>SCPI_BIND_NAME_MAX)
			len=SCPI_BIND_NAME_MAX;
		strncpy(this->response.name, name, SCPI_BIND_NAME_MAX);
	}
}

void SCPIBind::stop() {
	running=false;
	if (thread) {
		thread->join();
		delete thread;
		thread=0;
	}
}

void SCPIBind::run(void *arg) {
	info("starting scpi bind service");

	SCPIBind *binder=(SCPIBind *) arg;

	int sock=socket(AF_INET,SOCK_DGRAM,0);
	if(sock<0) {
		error("failed to create socket:%s", strerror(errno));
		return;
	}

	// setup endpoint
	struct sockaddr_in sock_addr;
	sock_addr.sin_family=AF_INET;
	sock_addr.sin_port=htons(SCPI_BIND_USERSPACE_PORT);
	sock_addr.sin_addr.s_addr = htonl(INADDR_ANY);
	if (bind(sock, (struct sockaddr *) &sock_addr, sizeof sock_addr)<0) {
		error("bind() failed: %s", strerror(errno));
		close(sock);
		return;
	}

	binder->running=true;
	for (;binder->running;) {
		struct sockaddr_in src_addr;
		socklen_t src_len;
		SCPIBindPackage pkt;
		size_t ret=recvfrom(sock, &pkt, sizeof pkt, 0, (struct sockaddr *) &src_addr, & src_len);

		if (ret>=8) {
			bool reply=false;
			switch(SCPIBindPackage::get_type(pkt)) {
			case 'P':
				debug("got 'P' package from ",inet_ntoa(src_addr.sin_addr));
				reply=true;
				break;
			case 'W': {
					debug("got 'W' package from ",inet_ntoa(src_addr.sin_addr));
					size_t namelen=pkt.name_length;
					if (namelen<(ret-8)) {
						reply=(strncmp(pkt.name, binder->response.name, pkt.name_length) == 0);
					}
				}
				break;
			default:
				break;
			}
			if (reply) {
				debug("sending reply");
				sendto(sock, &binder->response, sizeof binder->response, 0, (struct sockaddr *)&src_addr, src_len);
			}
		}
	}
	close(sock);
}
