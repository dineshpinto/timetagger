/*
 * BackendHttpd.h
 *
 *  Created on: 03.05.2013
 *      Author: mag
 */

#ifndef BACKENDHTTPD_H_
#define BACKENDHTTPD_H_

#include "ServerController.h"
#include "httpd/Httpd.h"

class ServerControl;

class BackendHttpd : public Httpd {
public:
	BackendHttpd(ServerControl *, jsVar *configuration);

protected:
	virtual int jsonRPC(std::string &method, JSON_FRAGMENT_STREAM &, jsVar *parameter);

private:
	ServerControl *server;

};


class jsonRpc : public JsonRequestHandler {
public:
	jsonRpc(BackendHttpd *);
	virtual int call(std::string &method, std::ostream &out, jsVar *params);

private:
	BackendHttpd *_httpd;
};

class jsonDownload : public CgiRequestHandler {
public:
	jsonDownload(BackendHttpd *);
	virtual int call(std::ostream &out, jsVar *params);

private:
	BackendHttpd *_httpd;
};

#endif /* BACKENDHTTPD_H_ */
