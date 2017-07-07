/*
 * SCPIBind.h
 *
 *  Created on: 22.03.2013
 *      Author: mag
 */

#ifndef SCPI_SCPIBIND_H_
#define SCPI_SCPIBIND_H_

#include <stdint.h>
#include <sys/types.h>
#include <sys/socket.h>
#include "../../backend/Thread.h"

#pragma pack(1)	// make sure, we byte-align..

static const int SCPI_BIND_NAME_MAX=32;
static const int SCPI_BIND_RETRY_COUNT=5;
static const int SCPI_BIND_PORT=888;
static const int SCPI_BIND_USERSPACE_PORT=8888;

struct SCPIBindPackage {
	unsigned char magic[3];
	unsigned char type;
	uint16_t version;
	uint16_t name_length;
	char name[SCPI_BIND_NAME_MAX];

	inline SCPIBindPackage() {
	}
	inline SCPIBindPackage(unsigned char tp) {
		magic[0]='p';
		magic[1]='i';
		magic[2]='b';
		magic[3]=tp;
		version=0x100;
		name_length=0;
	}

	inline static unsigned char get_type(const SCPIBindPackage &req) {
		if (req.magic[0]=='p' && req.magic[1]=='i' && req.magic[2]=='b')
			return req.type;
		return 0;
	}
};

#pragma pack()

#ifndef SCPI_BIND_CLIENT_ONLY

class SCPIBind {
public:
	SCPIBind(const char *name, int port=SCPI_BIND_PORT);
	~SCPIBind();
	void start();
	void stop();
protected:
	void prepare(const char *name);
	static void run(void *);

	bool running;
	SCPIBindPackage response;
	current_thread *thread;
};

#endif

#ifndef SCPI_BIND_SERVER_ONLY

class SCPIBindClient {
public:
	SCPIBindClient(int port=0);
	~SCPIBindClient();
	int resolve(const char *name);
	int query_all();
	void set_port(int);
protected:
	virtual bool on_answer(const SCPIBindPackage *, size_t pkglen, struct sockaddr *, socklen_t) = 0;

	int resolve(const SCPIBindPackage &query);
private:
	int port1, port2;
};

#endif

#endif /* SCPIBIND_H_ */

