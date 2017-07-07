/*
 * ServerControl.h
 *
 *  Created on: 24.04.2013
 *      Author: mag
 */

#ifndef SERVERCONTROL_H_
#define SERVERCONTROL_H_

#include <ostream>
#include <string>
#include <map>
#include "BackendController.h"
#include "BackendHttpd.h"
#include "Datastore.h"
#include "httpd/Httpd.h"
#include "error.h"
#include "../backend/HWClock.h"

class IdGenerator {
public:
	static std::string get_id() {
		char buffer[20];
		sprintf(buffer, "ID-%06x", ++id);
		return buffer;
	}

	inline static int get_max() {
		return id;
	}
	inline static void set_max(int _id) {
		id=_id;
	}
protected:
	static int id;
};

class Datastore;

class ServerControl {
public:
	ServerControl(const std::string config_file="timetaggerd.json");

	/* --- jsonrpc interface */
	int jsrpcGetConfiguration(JSON_FRAGMENT_STREAM&, jsVar *args);

	int jsrpcGetSystemInfo(JSON_FRAGMENT_STREAM&, jsVar *args);
	int jsrpcGetDeviceList(JSON_FRAGMENT_STREAM&, jsVar *args);
	int jsrpcScanDevices(JSON_FRAGMENT_STREAM&, jsVar *args);

	int jsrpcCreateDevice(JSON_FRAGMENT_STREAM&, jsVar *args);

	int jsonCreateEmulation(JSON_FRAGMENT_STREAM&);				// not used
	int jsrpcRemoveDevice(JSON_FRAGMENT_STREAM&, jsVar *args);


	/* --- implementations */
	BackendController *get_backend(const std::string &serial);
	BackendController *get_backend(JSON_FRAGMENT_STREAM &, jsVar *args);

	Datastore *get_datastore(JSON_FRAGMENT_STREAM &, jsVar *);

	void rescan_devices();

	void read_config();
	void read_config(const std::string &config_file);
	void write_config();
	void write_config(const std::string &config_file);
	void write_config(JSON_STREAM &out);

	BackendController *createBackendEmulation();	// not used
	void remove_backend(std::string &serial);	// not used

protected:
	Httpd *httpd;

	std::map<std::string,BackendController *>backends;

	HWClock start_time;
	std::string config_file;
	jsVar *config;

	Datastore *_datastore;
};

#endif /* SERVERCONTROL_H_ */
