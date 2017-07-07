/*
 * ServerControl.cpp
 *
 *  Created on: 24.04.2013
 *      Author: mag
 */

#include "ServerController.h"

#include "BackendHttpd.h"
#include "Datastore.h"

#include "../backend/Logger.h"

#include "../backend/FPGA.h"

#include <sstream>
#include <fstream>

#include <sys/file.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

static const char *VERSION_STRING="0.5beta";

int IdGenerator::id=0;

INSTALL_LOG_FACILITY(Logger::APPLICATION)

ServerControl::ServerControl(const std::string _config_file)
: config_file(_config_file) {

	info("starting timertaggerd, reading configuration from %s", _config_file.c_str());
	read_config(_config_file);

	// create datastore
	info("creating datastore");
	jsVar *cfg=config->getValue("datastore");
	_datastore=new Datastore(cfg);

	// create backends
	info("creating configured backends");
	cfg=config->getValue("backends");
	if (cfg && cfg->isObject()) {
		for (jsVar::iterator i=cfg->begin(); i!=cfg->end(); ++i) {
			std::string serial=i->first;
			jsVar *bc_cfg=i->second;
			BackendController *bc=new BackendController(serial, bc_cfg);
			backends[serial]=bc;
		}
	}

	// create httpd
	info("creating httpd");
	cfg=config->getValue("httpd");
	httpd=new BackendHttpd(this, cfg);
	info("httpd listening on %s", httpd->endpoints.c_str());

	// create scpid
	//TODO

	//write config
	write_config();
	info("timertaggerd startup complete");
}

void ServerControl::read_config() {
	read_config(config_file);
}

void ServerControl::write_config() {
	write_config(config_file);
}

void ServerControl::read_config(const std::string &filename) {
	config=0;
	std::ifstream cfg_file(config_file.c_str());
	if (cfg_file.good()) {
		cfg_file >> config;
		if (!config) {
			error("syntax error in configuration file.");
		}
	} else {
		error("configuration file not found.");
	}
	cfg_file.close();

	if (! config) {
		// create default configuration
		info("using default configuration");
		std::stringstream ss;
		ss << JSON(
					"backends" << JSEMPTY
				<<	"httpd" << JSON (
								"listening_ports" << 8080
							<<	"document_root" << "./timetaggerd/www"
						)
				);
		ss.seekg(0);
		ss >> config;
	}
}

void ServerControl::write_config(const std::string &filename) {
	int fd=open(config_file.c_str(), O_RDONLY);
	if (fd<0) {	// doesn't exist or error
		info("configfile not found, creating %s",filename.c_str() );
		fd=open(config_file.c_str(), O_WRONLY|O_CREAT, 0666);
	}
	if (fd>=0) {
		std::ofstream cfg(config_file.c_str());
		if (cfg.good()) {
			write_config(cfg);
			cfg << std::endl;
			cfg.flush();
		} else {
			error("writing configuration to %s failed.",filename.c_str());
		}
		close(fd);
	} else {
		// should i tell anybody?
		error("writing configuration to %s failed.",filename.c_str());
	}
	read_config(filename);
}

void ServerControl::write_config(std::ostream &cfg) {
	cfg << '{';

	cfg << JSKEY("datastore");
	_datastore->jsonWriteConfig(cfg);
	cfg << ',';

	cfg << JSKEY("httpd");
	httpd->jsonWriteConfig(cfg);
	cfg << ',';

	bool first=true;
	cfg << JSKEY("backends");
	cfg << '{';
	for (std::map<std::string,BackendController *>::iterator i=backends.begin(); i!=backends.end(); ++i) {
		if(!first) cfg << ',';
		first=false;
		JsonWriter::write_string(cfg, i->first);
		cfg << ":{";
		i->second->jsonWriteConfig(cfg);
		cfg << '}';
	}
	cfg << '}';

	cfg << '}';
	return;
}

Datastore *ServerControl::get_datastore(JSON_FRAGMENT_STREAM &, jsVar *) {
	return _datastore;
}

BackendController *ServerControl::get_backend(JSON_FRAGMENT_STREAM &json, jsVar *args) {
	std::string serial;
	if (args && args->getStr("device", serial)) {
		BackendController *bc=get_backend(serial);
		if (bc) return bc;

		json << "error" << JSON( "code" << 23 << "message" << "backend not found." );
		return 0;
	} else {
		json << "error:" << JSON( "code" << 3 << "message" << "bad device identifier" );
		return 0;
	}
}

int ServerControl::jsrpcGetConfiguration(JSON_FRAGMENT_STREAM &json, jsVar *args) {
	//TODO
	return HTTP_OK;
}

int ServerControl::jsrpcGetSystemInfo(JSON_FRAGMENT_STREAM &json, jsVar *args) {

	HWClock now=HWClock::now();
	long tm=now._secs-start_time._secs;

	int s=tm%60; tm/=60;
	int m=tm%60; tm/=60;
	int h=tm%24; tm/=24;
	int d=tm%365; tm/=365;
	int y=tm;

	std::stringstream ss;
	if (y>0) {
		ss << y << "y " << d << "d " << h << "h";
	} else if (d>0) {
		ss << d << "d " << h << "h " << m << "m";
	} else if (h>0) {
		ss << h << "h " << m << "m " << s <<"s";
	} else if (m>0) {
		ss << m << "m " << s <<"s";
	} else {
		ss << s <<"s";
	}

	json << "result"
		<< JSON(
				"version" << VERSION_STRING
			<<	"listener" << httpd->endpoints
			<<	"uptime" << ss.str()
		);
	return 200;
}

int ServerControl::jsrpcGetDeviceList(JSON_FRAGMENT_STREAM &json, jsVar *args) {
	int cnt=0;
	json << "result";

	JSON_STREAM &out=json.os;
	out << '[';
	for ( std::map<std::string,BackendController *>::iterator i=this->backends.begin(); i!=backends.end(); ++i) {
		if (cnt!=0)
			out << ',';
		++cnt;
		std::string serial=i->first;
		BackendController *bc=i->second;
		bc->jsonGetInfo(out);
	}
	out << "]";
	return 200;
}

std::string model(unsigned long brd) {
	switch (brd) {
		case 0x151f0023:	return "XEM3005";
		default:			return "<unknown>";
	}
}

int ServerControl::jsrpcScanDevices(JSON_FRAGMENT_STREAM &json, jsVar *args) {
	std::vector<FPGADevice> devices=FPGA::getDeviceList();

	json << "result";
	JSON_STREAM &out=json.os;
	out <<  '[';
	for (unsigned n=0; n<devices.size(); ++n) {
		if (n!=0)
			out << ',';
		FPGADevice &device=devices[n];
		out << JSON(
				"serial" << device.serial
			<< 	"hardware" << model(device.model)
			<< 	"id" << device.id
			<< "state" << (device.state ? "idle" : "running" )
		);
	}
	out << "]";
	return 200;
}

int ServerControl::jsrpcCreateDevice(JSON_FRAGMENT_STREAM &json, jsVar *args) {

	std::string serial;
	if (args && args->getStr("serial", serial)) {
		BackendController *bc=new BackendController(serial, args);
		backends[serial]=bc;
		std::string msg="device " + serial + " created";
		json << "result" <<  JSON( "message" << msg );
		return HTTP_OK;
	}
	json << "error" <<  JSON( "code" << 23 << "message" << "bad parameter" << "args" << args);
	return HTTP_OK;
}

int ServerControl::jsrpcRemoveDevice(JSON_FRAGMENT_STREAM &json, jsVar *args) {
	BackendController *bc=get_backend(json, args);
	if (bc) {
		std::string serial=bc->get_serial();
		backends.erase(serial);
		delete bc;

		json << "result" << JSEMPTY;
	}
	return 200;
}

BackendController *ServerControl::get_backend(const std::string &serial) {
	std::map<std::string,BackendController *>::iterator i=backends.find(serial);
	if (i!=backends.end())
		return i->second;
	return 0;
}

