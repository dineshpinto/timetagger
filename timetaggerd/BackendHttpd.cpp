/*
 * BackendHttpd.cpp
 *
 *  Created on: 03.05.2013
 *      Author: mag
 */

#include "BackendHttpd.h"

#include "httpd/json.h"
#include <iostream>

extern struct internal_http_files internal_files[];


BackendHttpd::BackendHttpd(ServerControl *ctl, jsVar *configuration)
: Httpd(configuration), server(ctl) {
#ifdef EMBEDD_HTDOCS
	this->set_internal_files(internal_files);
#endif
}

int BackendHttpd::jsonRPC(std::string &method, JSON_FRAGMENT_STREAM &out, jsVar *params) {
	if (method=="getConfiguration") {
		return server->jsrpcGetConfiguration(out, params);

	} else if (method=="getSystemInfo") {
		return server->jsrpcGetSystemInfo(out, params);

	} else if (method=="scanDevices") {
		return server->jsrpcScanDevices(out, params);

	} else if (method=="createDevice") {
		return server->jsrpcCreateDevice(out, params);

	} else if (method=="getDeviceList") {
		return server->jsrpcGetDeviceList(out, params);

	} else if (method=="removeDevice") {
		int ret=server->jsrpcRemoveDevice(out, params);
		server->write_config();
		return ret;

	} else if (method=="getDeviceInfo") {
		BackendController *bc;
		if ((bc=server->get_backend(out, params)))
			return bc->jsrpcGetInfo(out, params);
		else return BAD_REQUEST;

	} else if (method=="renameDevice") {
		BackendController *bc;
		if ((bc=server->get_backend(out, params))) {
			int ret=bc->jsrpcRename(out, params);
			server->write_config();
			return ret;
		}
		else return BAD_REQUEST;

	} else if (method=="getDACSettings") {
		BackendController *bc;
		if ((bc=server->get_backend(out, params)))
			return bc->jsrpcGetDACSettings(out, params);
		else return BAD_REQUEST;

	} else if (method=="setDACValue") {
		BackendController *bc;
		if ((bc=server->get_backend(out, params))) {
			int ret=bc->jsrpcSetDACValue(out, params);
			server->write_config();
			return ret;
		} else return BAD_REQUEST;

	} else if (method=="getFilter") {
		BackendController *bc;
		if ((bc=server->get_backend(out, params)))
			return bc->jsrpcGetFilter(out, params);
		else return BAD_REQUEST;

	} else if (method=="setFilter") {
		BackendController *bc;
		if ((bc=server->get_backend(out, params))) {
			int ret=bc->jsrpcSetFilter(out, params);
			server->write_config();
			return ret;
		} else return BAD_REQUEST;

	} else if (method=="getTestsignal") {
		BackendController *bc;
		if ((bc=server->get_backend(out, params)))
			return bc->jsrpcGetTestsignal(out, params);
		else return BAD_REQUEST;

	} else if (method=="setTestsignal") {
		BackendController *bc;
		if ((bc=server->get_backend(out, params)))
			return bc->jsrpcSetTestsignal(out, params);
		else return BAD_REQUEST;

	} else if (method=="getLineDelay") {
		BackendController *bc;
		if ((bc=server->get_backend(out, params)))
			return bc->jsrpcGetLineDelay(out, params);
		else return BAD_REQUEST;

	} else if (method=="setLineDelay") {
		BackendController *bc;
		if ((bc=server->get_backend(out, params)))
			return bc->jsrpcSetLineDelay(out, params);
		else return BAD_REQUEST;

	} else if (method=="getModuleClasses") {
		BackendController *bc;
		if ((bc=server->get_backend(out, params)))
			return bc->jsrpcGetClasses(out, params);
		else return BAD_REQUEST;

	} else if (method=="getModuleList") {
		BackendController *bc;
		if ((bc=server->get_backend(out, params)))
			return bc->jsrpcGetModules(out, params);
		else return BAD_REQUEST;

	} else if (method=="createModule") {
		BackendController *bc;
		if ((bc=server->get_backend(out, params))) {
			int ret=bc->jsrpcCreateModule(out, params);
			server->write_config();
			return ret;
		} else return BAD_REQUEST;

	} else if (method=="startModule") {
		BackendController *bc;
		if ((bc=server->get_backend(out, params)))
			return bc->jsrpcStartModule(out, params);
		else return BAD_REQUEST;

	} else if (method=="stopModule") {
		BackendController *bc;
		if ((bc=server->get_backend(out, params)))
			return bc->jsrpcStopModule(out, params);
		else return BAD_REQUEST;

	} else if (method=="clearModule") {
		BackendController *bc;
		if ((bc=server->get_backend(out, params)))
			return bc->jsrpcClearModule(out, params);
		else return BAD_REQUEST;

	} else if (method=="removeModule") {
		BackendController *bc;
		if ((bc=server->get_backend(out, params))) {
			int ret=bc->jsrpcRemoveModule(out, params);
			server->write_config();
			return ret;
		} else return BAD_REQUEST;

	} else if (method=="getModuleClass") {
		BackendController *bc;
		if ((bc=server->get_backend(out, params)))
			return bc->jsrpcGetModuleDescriptor(out, params);
		else return BAD_REQUEST;

	} else if (method=="getModuleConfig") {
		BackendController *bc;
		if ((bc=server->get_backend(out, params)))
			return bc->jsrpcGetModuleConfig(out, params);
		else return BAD_REQUEST;

	} else if (method=="setModuleConfig") {
		BackendController *bc;
		if ((bc=server->get_backend(out, params))) {
			int ret=bc->jsrpcSetModuleConfig(out, params);
			server->write_config();
			return ret;
		} else return BAD_REQUEST;

	} else if (method=="getData") {
		BackendController *bc;
		if ((bc=server->get_backend(out, params)))
			return bc->jsrpcGetData(out, params);
		else return BAD_REQUEST;



	} else if (method=="getDatasetList") {
		Datastore *ds;
		if ((ds=server->get_datastore(out, params)))
			return ds->jsrpcGetDatasetList(out, params);
		else return BAD_REQUEST;

	} else if (method=="createDataset") {
		Datastore *ds;
		if ((ds=server->get_datastore(out, params)))
			return ds->jsrpcCreateDataset(out, params);
		else return BAD_REQUEST;

	} else if (method=="removeDataset") {
		Datastore *ds;
		if ((ds=server->get_datastore(out, params)))
			return ds->jsrpcRemoveDataset(out, params);
		else return BAD_REQUEST;

	} else if (method=="getDataset") {
		Datastore *ds;
		if ((ds=server->get_datastore(out, params)))
			return ds->jsrpcGetDataset(out, params);
		else return BAD_REQUEST;

	} else if (method=="downloadDataset") {
		Datastore *ds;
		if ((ds=server->get_datastore(out, params)))
			return ds->jsrpcDownloadDataset(out, params);
		else return BAD_REQUEST;

	} else if (method=="saveDataset") {
		Datastore *ds;
		if ((ds=server->get_datastore(out, params)))
			return ds->jsrpcSaveDataset(out, params);
		else return BAD_REQUEST;


	} else
		return NOT_FOUND;
}
