/*
 * scpi.h
 *
 *  Created on: 15.03.2013
 *      Author: mag
 */

#ifndef SCPI_H_
#define SCPI_H_

#include <string>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include "../../backend/Thread.h"
#include "libscpi/scpi.h"

class SCPIContext;
class SCPIServer;
class SCPISocket;

const int DEFAULT_BUFFERSIZE=1024;
const int DEFAULT_TIMEOUT=1000;


class SCPI {
public:
	SCPI();
	virtual ~SCPI();

	void open_server(int port);
	void shutdown();

	virtual SCPIContext *new_context();

	virtual int error(scpi_t *, int_fast16_t err);
	virtual scpi_result_t test(scpi_t *);
	virtual scpi_result_t reset(scpi_t *);
	virtual scpi_result_t control (scpi_t * context, unsigned ctrl, unsigned val);

public:
	bool register_server(SCPIServer *);
	bool unregister_server(SCPIServer *);

	SCPIServer **server_list;
	size_t server_list_len;
	current_mutex server_list_mutex;
};

class SCPIContext : public scpi_t {
public:
	SCPIContext(SCPI *);
	virtual ~SCPIContext();


	bool getParamInt(int *val, bool mandatory);
	bool getParam(const char **str, size_t *sz, bool mandatory);
	bool getParamNumber(scpi_number_t *num, bool mandatory);
	bool getParamText(const char **str, size_t *sz, bool mandatory);

	bool parseParamInt(int *val, const char *str, size_t sz);
	bool parseParamNumber(scpi_number_t *num, const char *str, size_t sz);

	size_t strToInt(long *, const char *, size_t);
	size_t strToDouble(double *, const char *, size_t);

public:
	scpi_result_t execute(const char *, size_t);

	void set_socket(int port, struct sockaddr_in addr);

	SCPI *scpi;
	size_t do_read(char *, size_t, int);
	size_t do_write(const char *, size_t);
	scpi_result_t do_flush();

protected:
	int in_fd;
	int out_fd;
	bool use_recv;
	struct sockaddr_in remote_addr;
};


class SCPIServer {
public:
	SCPIServer(SCPI *, int port);
	~SCPIServer();

	virtual void start();
	void stop();
	void terminate();

	bool register_socket(SCPISocket *);
	bool unregister_socket(SCPISocket *);

	SCPI *get_scpi();
protected:
	static void run(void *);

	int open_socket();

	void register_connection(SCPISocket *);
	void unregister_connection(SCPISocket *);


	SCPI *scpi;
	int port;
	bool running;
	int listener;

	current_thread *thread;

	SCPISocket **socket_list;
	size_t socket_list_len;
	current_mutex socket_list_mutex;
};

class SCPISocket {
public:
	SCPISocket(SCPIServer *, int fd, struct sockaddr_in addr);
	virtual ~SCPISocket();

	virtual void start();
	void stop();
	void terminate();

protected:
	static void run(void *);

	void scpi_init();
	void scpi_finit();
	void scpi_execute(const char*, size_t);

	static size_t write_to_context(scpi_t *context, const char *buffer, size_t buffersize);

	SCPIServer *server;

	int fd;

	struct sockaddr_in remote_addr;

	bool running;
	int timeout;

	current_thread *thread;
};


#endif /* SCPI_H_ */
