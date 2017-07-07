/*
 * SCPIResolve.cpp
 *
 *  Created on: 23.03.2013
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

#define SCPI_BIND_CLIENT_ONLY

#include "SCPIBind.h"

#include "../../backend/Logger.h"

SCPIBindClient::SCPIBindClient(int port) {
	if (port==0) {
		port1=SCPI_BIND_PORT;
		port2=SCPI_BIND_USERSPACE_PORT;
	} else {
		port1=port;
		port2=0;
	}
}

SCPIBindClient::~SCPIBindClient() {
}


void SCPIBindClient::set_port(int port) {
	port1=port; port2=0;
}

int SCPIBindClient::resolve(const char *name) {
	SCPIBindPackage pkg=SCPIBindPackage('W');
	int len=strlen(name);
	if (len>SCPI_BIND_NAME_MAX)
		len=SCPI_BIND_NAME_MAX;
	return resolve(pkg);
}

int SCPIBindClient::query_all() {
	SCPIBindPackage pkg=SCPIBindPackage('P');
	return resolve(pkg);
}

int SCPIBindClient::resolve(const SCPIBindPackage &query) {

	// create socket
	int sock=socket(AF_INET,SOCK_DGRAM,0);
	if(sock<0) {
		//TODO: logg error
		return 0;
	}

	// enable broadcast
	int broadcast_flag=1;
	if (setsockopt(sock,SOL_SOCKET, SO_BROADCAST,(char *)&broadcast_flag,sizeof(broadcast_flag))<0) {
		//TODO: logg error
		return 0;
	}

	// listen, also sets random port number
	struct sockaddr_in sock_addr;
	sock_addr.sin_family=AF_INET;
	sock_addr.sin_port=htons(0);
	sock_addr.sin_addr.s_addr = htonl(INADDR_ANY);
	if (bind(sock, (struct sockaddr *) &sock_addr, sizeof sock_addr)<0) {
		//TODO: logg error
		close(sock);
		return 0;
	}

	// setup recv timeout to 500msec
	struct timeval tv;
	tv.tv_sec = 0;
	tv.tv_usec=1000 * 500;
	if (setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, (char *)&tv,  sizeof tv)) {
		//TODO: logg error
		return 0;
	}

	// setup broadcast addr
	struct sockaddr_in bcast_addr;
	bcast_addr.sin_family=AF_INET;
	bcast_addr.sin_addr.s_addr = htonl(INADDR_BROADCAST);

	for (int resend=0; resend<SCPI_BIND_RETRY_COUNT; ++resend) {


		sockaddr sender_addr;
		socklen_t sender_len;
		char buffer[4096];
		size_t sz;

		if (port1>0) {
			bcast_addr.sin_port=htons(port1);
			sendto(sock, &query, sizeof query, 0, (struct sockaddr *) &bcast_addr, sizeof bcast_addr);
			bcast_addr.sin_port=htons(port1);
			sz=recvfrom(sock, buffer, 4096, 0, &sender_addr, &sender_len);
			if (sz>0) {
				if (on_answer((SCPIBindPackage *)buffer, sz, (struct sockaddr *)&sender_addr, sender_len)!=0) {
					close(sock);
					return true;
				}
			}
		}

		if (port2>0) {
			bcast_addr.sin_port=htons(port1);
			sendto(sock, &query, sizeof query, 0, (struct sockaddr *) &bcast_addr, sizeof bcast_addr);
			bcast_addr.sin_port=htons(port2);
			sz=recvfrom(sock, buffer, 4096, 0, &sender_addr, &sender_len);
			if (sz>0) {
				if (on_answer((SCPIBindPackage *)buffer, sz, (struct sockaddr *)&sender_addr, sender_len)!=0) {
					close(sock);
					return true;
				}
			}
		}
		sleep(2);
	}
	close(sock);
	return false;

	//TODO
}
