/*
 * main.cpp
 *
 *  Created on: 23.04.2013
 *      Author: mag
 */

#include <iostream>
#include <sstream>

#include <string.h>
#include <memory.h>
#include <unistd.h>
#include <stdlib.h>

#include "mongoose.h"
#include "json.h"
#include "HttpStream.h"

static int rpc_handler(struct mg_connection *conn) {

	const struct mg_request_info *req = mg_get_request_info(conn);

	//TODO: do logging

	if (strcmp(req->uri, "/json-rpc")==0) {
		MongooseInbuffer inbuf(conn);
		MongooseOutbuffer outbuf(conn);
		std::istream in(&inbuf);
		std::ostream out(&outbuf);

		jsVar *request;
		in >> request;

		out	<< 	"HTTP/1.1 200 OK\r\n"
			<<	"Content-Type: application/json\r\n\r\n"
			<<	"{\"jsonrpc\":\"2.0\",";
		if (request) {
			std::string proto, method;
			jsVar *id;
			//TODO: parse params
			jsVar *params=0;
			request->getStr("jsonrpc",proto);
			bool ret=request->getStr("method",method);

			id=request->getValue("id");
			if (id) {
				out << "\"id\":" << id << ",";
			}

			if (!ret || proto!="2.0") {
				out << "\"error\": { \"code\": -32600, \"message\":\"invalid request\"}";
			} else if (!params || !params->isArray() || !params->isObject()) {
				out << "\"error\": { \"code\": -32600, \"message\":\"invalid request\","
			          "\"data\":\"missing parameter\"}";

			} else {

				// syntax ok, execute method..

				out << "\"error\": { \"code\": -32601, \"message\":\"method not found\","
			          "\"data\":\"unknown method " << method << "\"}";
			}
			delete request;
		}
		out << "}";

		out.flush();
		return 1;
	}

	return 0;
}

int main(void) {
  struct mg_context *ctx;
  const char *options[] = {"listening_ports", "8080", "document_root", "./www", NULL};

  struct mg_callbacks callbacks;
  memset(&callbacks, 0, sizeof(callbacks));
  callbacks.begin_request = rpc_handler;

  ctx = mg_start(&callbacks, NULL, options);
  getchar();  // Wait until user hits "enter"
  mg_stop(ctx);
  return 0;
}
