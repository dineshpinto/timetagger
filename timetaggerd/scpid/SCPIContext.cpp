/*
 * SCPIContext.cpp
 *
 *  Created on: 19.03.2013
 *      Author: mag
 */


#include <unistd.h>
#include <string.h>
#include <errno.h>

#include "SCPI.h"

#include "../../backend/Logger.h"


static void error(const char *fmt, ...) {
	va_list ap;
	va_start(ap, fmt);
	Logger::vlog( Logger::APPLICATION, Logger::LOG_ERROR, fmt, ap);
}


static int forward_error(scpi_t * context, int_fast16_t err) {
	return ((SCPIContext *)context)->scpi->error(context, err);
}
static scpi_result_t forward_test(scpi_t * context) {
	return ((SCPIContext *)context)->scpi->test(context);
}

static scpi_result_t forward_reset(scpi_t * context) {
	return ((SCPIContext *)context)->scpi->reset(context);
}

static scpi_result_t forward_control(scpi_t * context, scpi_ctrl_name_t ctrl, scpi_reg_val_t val) {
	return ((SCPIContext *)context)->scpi->control(context, (unsigned) ctrl, (unsigned) val);
}

static scpi_result_t forward_flush(scpi_t *context) {
	return ((SCPIContext*)(context))->do_flush();
}

static size_t forward_write(scpi_t *context, const char *data, size_t len) {
	return ((SCPIContext*)(context))->do_write(data, len);
}

const int register_count=SCPI_REG_COUNT;	//TODO:

static const scpi_command_t dummy_scpi_commands[] = {
	    { 0,0 }
};

SCPIContext::SCPIContext(SCPI *_scpi)
: scpi(_scpi),
  in_fd(STDIN_FILENO), out_fd(STDOUT_FILENO), use_recv(false) {
	cmdlist = dummy_scpi_commands; // will be overriden in subclass

	buffer.length=DEFAULT_BUFFERSIZE*2;
	buffer.data = new char[buffer.length];

	interface = new _scpi_interface_t;
	interface->error=forward_error;
	interface->reset=forward_reset;
	interface->test=forward_test;
	interface->control=forward_control;

	interface->flush=forward_flush;
	interface->write=forward_write;

	registers = new scpi_reg_val_t[register_count];

	units = scpi_units_def;
	special_numbers = scpi_special_numbers_def;

	SCPI_Init(this);
}


SCPIContext::~SCPIContext() {
	delete interface;
	delete [] buffer.data;
	delete [] registers;
}

size_t SCPIContext::do_read(char *buffer, size_t len, int timeout) {
	if (!use_recv)
		return ::read(in_fd, buffer, len);
	else {
		int fd=this->in_fd;
		fd_set rfds;
		struct timeval tv;
		int retval;

		FD_ZERO(&rfds);
		FD_SET(fd, &rfds);
		tv.tv_sec = timeout / 1000;
		tv.tv_usec = timeout % 1000;

		retval = select(fd+1, &rfds, NULL, NULL, &tv);
		if (retval<0) {
		   error("error on stream read, select got (%s)", strerror(errno));
		   return -1;

		} else if (retval>=0) {
			retval = recv(fd, buffer, len,0);
			if (errno == EAGAIN)
				return 0;
			else if (retval==0)
				return -1;
			else
				return retval;

		} else {
			return -1;
		}

	}
}

size_t SCPIContext::do_write(const char *buffer, size_t len) {
	if (!use_recv) {
		size_t sz=0;
		while (sz<len) {
			int ret=::write(out_fd, buffer+sz, len-sz);
			if (ret<=0) {
				return -1;
			}
			sz+=ret;
		}
		return sz;
	} else {
		size_t sz=0;
		while (sz<len) {
			int ret=::send(out_fd, buffer+sz, len-sz,0);
			if (ret<=0) {
				return -1;
			}
			sz+=ret;
		}
		return sz;
	}
}

scpi_result_t SCPIContext::do_flush() {
	return SCPI_RES_OK;
}

scpi_result_t SCPIContext::execute(const char *buffer, size_t len) {
	SCPI_Input(this, buffer, len);
	return SCPI_RES_OK;
}

void SCPIContext::set_socket(int fd, struct sockaddr_in addr) {
	use_recv=true;
	in_fd=out_fd=fd;
	remote_addr=addr;
}

bool SCPIContext::getParamInt(int *val, bool mandatory) {
	int32_t ival;
	bool ret=SCPI_ParamInt(this, &ival, mandatory);
	*val=ival;
	return ret;
}

bool SCPIContext::getParam(const char **str, size_t *sz, bool mandatory) {
	return SCPI_ParamString(this, str, sz, mandatory);
}

bool SCPIContext::parseParamInt(int *val, const char *str, size_t sz) {
	return SCPI_ParseParamInt(this, val, str, sz);
}
bool SCPIContext::parseParamNumber(scpi_number_t *num, const char *str, size_t sz) {
	return SCPI_ParseParamNumber(this, num, str, sz);
}





