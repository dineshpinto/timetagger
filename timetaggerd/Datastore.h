/*
 * Datastore.hpp
 *
 *  Created on: 07.05.2013
 *      Author: mag
 */

#ifndef DATASTORE_H_
#define DATASTORE_H_

#include "httpd/json.h"
#include <ostream>
#include <string>
#include "ServerController.h"

class Datastore {
public:

	Datastore(jsVar *config);

	int jsrpcGetDatasetList(JSON_FRAGMENT_STREAM &out, jsVar *param);
	int jsrpcCreateDataset(JSON_FRAGMENT_STREAM &out, jsVar *param);
	int jsrpcSaveDataset(JSON_FRAGMENT_STREAM &out, jsVar *param);
	int jsrpcGetDataset(JSON_FRAGMENT_STREAM &out, jsVar *param);
	int jsrpcDownloadDataset(JSON_FRAGMENT_STREAM &out, jsVar *param);
	int jsrpcRemoveDataset(JSON_FRAGMENT_STREAM &out, jsVar *param);

	void jsonWriteConfig(JSON_STREAM &);
	void writeCatalog();

	static std::string get_datapath();
	static std::string get_dumppath();

protected:

	std::string data_path;
	std::string catalog_path;
	jsVar *catalog;
};

#endif /* DATASTORE_H_ */
