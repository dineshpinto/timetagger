/*
 * Httpd.cpp
 *
 *  Created on: 27.04.2013
 *      Author: mag
 */

#include "Httpd.h"
#include "HttpStream.h"

#include "../../backend/Logger.h"
#include "../ServerController.h"


#include <stdlib.h>
#include <stdio.h>
#include <string>
#include <map>
#include <string.h>
#include <iostream>
#include <sstream>
#include <time.h>



int Httpd::__begin_request(struct mg_connection *conn) {
	const struct mg_request_info *req = mg_get_request_info(conn);
	void *userdata=req->user_data;
	Httpd *httpd=static_cast<Httpd *>(userdata);
	if (httpd) return httpd->begin_request(conn);
	else return 0;
}
void Httpd::__end_request(const struct mg_connection *c, int reply_status_code) {
	struct mg_connection *conn=(struct mg_connection *)c;
	struct mg_request_info *req = mg_get_request_info(conn);
	void *userdata=req->user_data;
	Httpd *httpd=static_cast<Httpd *>(userdata);
	if (httpd) httpd->end_request(conn, reply_status_code);
}
int Httpd::__log_message(const struct mg_connection *c, const char *message) {
	struct mg_connection *conn=(struct mg_connection *)c;
	struct mg_request_info *req = mg_get_request_info(conn);
	void *userdata=req->user_data;
	Httpd *httpd=static_cast<Httpd *>(userdata);
	if (httpd) return httpd->log_message(conn, message);
	else return 0;
}

const char *Httpd::__open_file(const struct mg_connection *c, const char *path, size_t *datalen) {
	struct mg_connection *conn=(struct mg_connection *)c;
	struct mg_request_info *req = mg_get_request_info(conn);
	void *userdata=req->user_data;
	Httpd *httpd=static_cast<Httpd *>(userdata);
	if (httpd) return httpd->open_file(conn, path, datalen);
	else return 0;
}

const char *Httpd::open_file(struct mg_connection *c, const char *path, size_t *datalen) {

	if (_internal_files) {
		const char *root=mg_get_option(ctx, "document_root");
		path += strlen(root);

		struct internal_http_files *p=_internal_files;
		while (p->path) {
			if (strcmp(path, p->path)==0) {
				*datalen=p->size;
				return p->chars;
			}
			++p;
		}
	}
	return 0;
}

Httpd::Httpd(jsVar *cfg) : _internal_files(0) {

	request_handler=0;

	if (cfg->isObject()) {
		for (jsVar::iterator i=cfg->begin(); i!=cfg->end(); ++i) {
			const std::string key=i->first;
			jsVar *val=i->second;
			if (val && val->isAtomic()) {
				configuration[key] = val->asString();
			}
		}
	}

	const char **options=new const char *[configuration.size()*2+2];
	int n=0;
	for (std::map<std::string, std::string>::iterator i=configuration.begin(); i!=configuration.end(); ++i) {
		options[n++]=i->first.c_str();
		options[n++]=i->second.c_str();
	}
	options[n++]=0;


	struct mg_callbacks callbacks;
	memset(&callbacks, 0, sizeof(callbacks));

	callbacks.begin_request = __begin_request;
	callbacks.end_request=__end_request;
	callbacks.log_message=__log_message;
	callbacks.open_file=__open_file;

	ctx = mg_start(&callbacks, this, options);

	if(!ctx) {
		Logger::log(Logger::HTTP, Logger::LOG_FATAL,"couldn't start webserver");
		exit(1);
	}

	local_ips=mg_local_ips(ctx);
	std::stringstream ss;
	for (char **p=local_ips; *p; ++p) {
		if (p!=local_ips) ss << ", ";
		ss << *p;
	}
	endpoints=ss.str();
}

Httpd::~Httpd() {
	stop();
}

void Httpd::start() {
}

void Httpd::stop() {
}

void Httpd::jsonWriteConfig(std::ostream &out) {
	bool first=true;
	out << '{';
	for (std::map<std::string, std::string>::iterator i=configuration.begin(); i!=configuration.end(); ++i) {
		if(!first) out << ',';
		first=false;
		out << '\"' << i->first << "\":" << '\"' << i->second << '\"';
	}
	out << '}';
}

void Httpd::addRequesthandler(const char *mountpoint, HttpRequestHandler *handler) {
	struct rq_list *rq=new rq_list;
	rq->mountpoint=strdup(mountpoint);
	rq->mplen=strlen(mountpoint);
	rq->handler=handler;
	rq->next=this->request_handler;
	this->request_handler=rq;
}

void Httpd::set_internal_files(struct internal_http_files *files) {
	this->_internal_files=files;
}

int Httpd::begin_request(struct mg_connection *conn) {
	struct mg_request_info *req = mg_get_request_info(conn);

	struct rq_list *head=this->request_handler;
	while (head) {


		if (strncmp(req->uri, head->mountpoint, head->mplen)==0) {
			MongooseInbuffer inbuf(conn);
			MongooseOutbuffer outbuf(conn);
			std::istream in(&inbuf);
			std::ostream out(&outbuf);

			req->uri += head->mplen;
			return head->handler->do_request(conn, req, in, out);
		}
		head=head->next;
	}

	if (strcmp(req->uri, "/")==0) {
		MongooseInbuffer inbuf(conn);
		MongooseOutbuffer outbuf(conn);
		std::istream in(&inbuf);
		std::ostream out(&outbuf);

		out	<< 	"HTTP/1.1 301 Moved Permanently\r\n"
			<<	"Location: /index.shtml\r\n\r\n";
		out.flush();
		return 1;

	}

	if (strcmp(req->uri, "/json-rpc")==0) {
		MongooseInbuffer inbuf(conn);
		MongooseOutbuffer outbuf(conn);
		std::istream in(&inbuf);
		std::ostream out2(&outbuf);
		std::ostringstream out;
		struct timespec now, tmstart;
		clock_gettime(CLOCK_REALTIME, &tmstart);

		jsVar *request;
		in >> request;
		std::cout << "json-rpc call, param:" << request << std::endl;

		out2	<< 	"HTTP/1.1 200 OK\r\n"
			<<	"Content-Type: application/json\r\n\r\n"
			;

		JSON_FRAGMENT_STREAM json= (out << ObjectWriter() << "jsonrpc" << "2.0");

		if (request) {
			std::string proto, method;
			jsVar *id;
			jsVar *params=0;
			request->getStr("jsonrpc",proto);
			bool ret=request->getStr("method",method);

			id=request->getValue("id");
			if (id) {
				json << "id" << id;
			}
			params=request->getValue("params");
			if (!ret || proto!="2.0") {
				json	<< "error"
						<< JSON( 	"code" << ERR_INVALID_REQUEST
								<< 	"message" << MSG_INVALID_REQUEST
								<< 	"data" << "bad protocol" );

			} else if (params && *params && (!params->isArray()  && !params->isObject()) ) {
				json 	<< "error"
						<< JSON( 	"code" << ERR_INVALID_REQUEST
								<< 	"message" << MSG_INVALID_REQUEST
								<< 	"data" << "bad parameter" );

			} else {

				// syntax ok, execute method..

				if (jsonRPC(method, json, params)==NOT_FOUND) {
					json 	<< "error"
							<< JSON( 	"code" << ERR_METHOD_NOT_FOUND
									<< 	"message" << MSG_METHOD_NOT_FOUND
									<< 	"data" << method );
				}
			}
			delete request;
		}

		clock_gettime(CLOCK_REALTIME, &now);
		double seconds = (double)((now.tv_sec+now.tv_nsec*1e-9) - (double)(tmstart.tv_sec+tmstart.tv_nsec*1e-9));
		json << "time" << seconds;

		json <<  ObjectWriter::endObject();

		std::cout << "json-rpc answer: " << out.str() << std::endl << std::endl;
		out2 << out.str();
		out2.flush();
		return 1;
	}

	return 0;
}

int Httpd::jsonRPC(std::string &method, JSON_FRAGMENT_STREAM &out, jsVar *params) {
	return NOT_FOUND;
}


int Httpd::end_request(struct mg_connection *conn, int reply_code) {
	//const struct mg_request_info *req = mg_get_request_info(conn);
	//Logger::log(Logger::HTTP, Logger::INFO, "%d - %s", reply_code, req->uri);
	return 0;
}

int Httpd::log_message(struct mg_connection *conn, const char *message) {
	Logger::log(Logger::HTTP, Logger::LOG_INFO, message);
	return 0;
}



int HttpRequestHandler::do_request(std::ostream &out) {
	return 404;
}

int HttpRequestHandler::do_request(std::istream &in, std::ostream &out) {
	return do_request(out);
}

int HttpRequestHandler::do_request(const struct mg_request_info *, std::ostream &out) {
	return do_request(out);
}

int HttpRequestHandler::do_request(struct mg_connection *conn, const struct mg_request_info *, std::ostream &out) {
	return do_request(out);
}

int HttpRequestHandler::do_request(const struct mg_request_info *, std::istream &, std::ostream &out) {
	return do_request(out);
}

int HttpRequestHandler::do_request(struct mg_connection *conn, const struct mg_request_info *, std::istream &, std::ostream &out) {
	return do_request(out);
}


int JsonRequestHandler::do_request(struct mg_connection *conn, struct mg_request_info *, std::istream &in, std::ostream &out) {
	jsVar *request;
	in >> request;

	out	<< 	"HTTP/1.1 200 OK\r\n"
		<<	"Content-Type: application/json\r\n\r\n"
		<<	"{\"jsonrpc\":\"2.0\",";
	if (request) {
		std::string proto, method;
		jsVar *id;
		jsVar *params=0;
		request->getStr("jsonrpc",proto);
		bool ret=request->getStr("method",method);

		id=request->getValue("id");
		if (id) {
			out << "\"id\":" << id << ",";
		}
		params=request->getValue("params");

		if (!ret || proto!="2.0") {
			out << "error"
				<< JSON( 	"code" << -32600
						<< 	"message" << "invalid request"
						<< 	"data" << "bad protocol" );
		} else if (0 && params && (!params->isArray()  || !params->isObject()) ) {
			out << "error"
				<< JSON( 	"code" << -32600
						<< 	"message" << "invalid request"
						<< 	"data" << "bad parameter" );

		} else {

			// syntax ok, execute method..

			if (call(method, out, params)==Httpd::NOT_FOUND) {
				out << "error"
					<< JSON( 	"code" << -32601
							<< 	"message" << "method not found"
							<< 	"data" << method );
			}
		}
		delete request;
	} else {
		out << "error"
			<< JSON( 	"code" << -32600
					<< 	"message" << "invalid request"
					<< 	"data" << "bad protocol" );
	}
	out << "}" << std::endl;

	out.flush();
	return 1;
}

int JsonRequestHandler::call(std::string &method, std::ostream &, jsVar *) {
	return 404;
}

int RestRequestHandler::do_request(struct mg_connection *conn, struct mg_request_info *, std::istream &, std::ostream &) {
	//TODO
	return 404;
}

int RestRequestHandler::call(std::string &method, std::ostream &, jsVar *) {
	return 404;
}

int CgiRequestHandler::do_request(struct mg_connection *conn, struct mg_request_info *, std::istream &, std::ostream &) {
	//TODO
	return 404;
}

int CgiRequestHandler::call(std::ostream &, jsVar *) {
	return 404;
}

