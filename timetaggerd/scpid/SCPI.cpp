/*
 * SCPI.cpp
 *
 *  Created on: 15.03.2013
 *      Author: mag
 */


#include <string.h>
#include <stdlib.h>
#include <errno.h>

#include "../../backend/Logger.h"

#include "SCPI.h"


SCPI::SCPI() {
	server_list_len=4;
	server_list=(SCPIServer **)calloc(sizeof(SCPIServer **), server_list_len);
}

SCPI::~SCPI() {
	shutdown();
	free(server_list);
}

SCPIContext *SCPI::new_context() {
	return new SCPIContext(this);
}

void SCPI::open_server(int port) {
	SCPIServer *server=new SCPIServer(this, port);
	server->start();
}

bool SCPI::register_server(SCPIServer *s) {
	server_list_mutex.lock();
	size_t n;
	for (n=0; n<server_list_len; ++n) {
		if (server_list[n]==0)
			server_list[n]=s;
			server_list_mutex.unlock();
			return true;
	}
	size_t len=server_list_len;
	len+=len/2;
	SCPIServer **list=(SCPIServer **)realloc(server_list, sizeof (SCPIServer**)*len);
	if (!list) {
		server_list_mutex.unlock();
		return false;
	}
	for (n=0; n<len; ++n) {
		if (n<server_list_len) {
			list[n]=server_list[n];
		} else if (n==server_list_len) {
			list[n]=s;
		} else {
			list[n]=0;
		}
	}
	server_list=list;
	server_list_len=len;
	server_list_mutex.unlock();
	return true;
}

bool SCPI::unregister_server(SCPIServer *s) {
	server_list_mutex.lock();
	for (size_t n=0; n<server_list_len; ++n) {
		if (server_list[n]==s) {
			server_list[n]=0;
			server_list_mutex.unlock();
			return false;
		}
	}
	server_list_mutex.unlock();
	return false;
}

void SCPI::shutdown() {
	for (size_t n=0; n<server_list_len; ++n) {
		if (server_list[n]) {
			server_list[n]->stop();
		}
	}
	for (size_t n=0; n<server_list_len; ++n) {
		if (server_list[n]) {
			server_list[n]->terminate();
		}
	}
}


int SCPI::error(scpi_t* context, int_fast16_t err) {
	return 0;
}

scpi_result_t SCPI::test(scpi_t * context) {
	return ((SCPI *)context->user_context)->test(context);
	return SCPI_RES_OK;
}

scpi_result_t SCPI::reset(scpi_t * context) {
	return SCPI_RES_OK;
}

scpi_result_t SCPI::control (scpi_t * context, unsigned ctrl, unsigned val) {
	return SCPI_RES_OK;
}
