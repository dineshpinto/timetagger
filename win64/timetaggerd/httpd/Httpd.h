/*
 * Httpd.h
 *
 *  Created on: 27.04.2013
 *      Author: mag
 */

#ifndef HTTPD_H_
#define HTTPD_H_

#include "json.h"
#include "mongoose.h"

class HttpRequestHandler;

struct internal_http_files {
	const char *path;
	unsigned size;
	const char *chars;
};

class Httpd {
public:
	static const int NOT_FOUND = 404;
	static const int BAD_REQUEST = 400;
	static const int HTTP_OK = 400;


public:
	Httpd(jsVar *configuration);
	~Httpd();

	void start();
	void stop();

	void jsonWriteConfig(std::ostream &cfg);

	void addRequesthandler(const char *mountpoint, HttpRequestHandler *);
	void set_internal_files(struct internal_http_files *files);

protected:
	virtual int jsonRPC(std::string &method, JSON_FRAGMENT_STREAM &, jsVar *parameter);

	int begin_request(struct mg_connection *conn);
	int end_request(struct mg_connection *conn, int);
	int log_message(struct mg_connection *conn, const char *);
	const char *open_file(struct mg_connection *c, const char *path, size_t *datalen);

private:
	static int __begin_request(struct mg_connection *conn);
	static void __end_request(const struct mg_connection *conn, int);
	static int __log_message(const struct mg_connection *conn, const char *);
	static const char *__open_file(const struct mg_connection *c, const char *path, size_t *datalen);

	struct mg_context *ctx;

	struct rq_list {
		int mplen;
		const char *mountpoint;
		HttpRequestHandler *handler;
		struct rq_list *next;
	} *request_handler;

	struct internal_http_files *_internal_files;

public:
	std::map<std::string, std::string> configuration;
	char **local_ips;
	std::string endpoints;
};

class HttpRequestHandler {
public:
	virtual int do_request(std::ostream &out);
	virtual int do_request(std::istream &, std::ostream &out);
	virtual int do_request(const struct mg_request_info *, std::ostream &);
	virtual int do_request(struct mg_connection *conn, const struct mg_request_info *, std::ostream &);
	virtual int do_request(const struct mg_request_info *, std::istream &, std::ostream &);
	virtual int do_request(struct mg_connection *conn, const struct mg_request_info *, std::istream &, std::ostream &);
};

class CgiRequestHandler : public HttpRequestHandler {
public:
	virtual int do_request(struct mg_connection *conn, struct mg_request_info *, std::istream &, std::ostream &);

	virtual int call(std::ostream &, jsVar *);
};

class JsonRequestHandler : public HttpRequestHandler {
public:
	virtual int do_request(struct mg_connection *conn, struct mg_request_info *, std::istream &, std::ostream &);

	virtual int call(std::string &method, std::ostream &, jsVar *);
};

class RestRequestHandler : public HttpRequestHandler {
public:
	virtual int do_request(struct mg_connection *conn, struct mg_request_info *, std::istream &, std::ostream &);

	virtual int call(std::string &method, std::ostream &, jsVar *);
};

#endif /* HTTPD_H_ */
