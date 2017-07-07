/*
 * Datastore.cpp
 *
 *  Created on: 07.05.2013
 *      Author: mag
 */

#include "Datastore.h"

#include <sys/types.h>
#include <dirent.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdlib.h>
#include <limits.h>

#include <sstream>
#include <fstream>
#include <iostream>

#include "DiagramData.h"

#ifdef WIN32
#define MKDIR(path,mode) mkdir(path)
#else
#define MKDIR(path,mode) mkdir(path,mode)
#endif

static void ensure_directory(std::string path) {
	struct stat s_stat;
	if (stat(path.c_str(), &s_stat)!=0) {
		MKDIR(path.c_str(),0750);
		if (stat(path.c_str(), &s_stat)!=0) {
			perror("couldn't create application data directory");
			abort();
		}
	}
	if (!(s_stat.st_mode&S_IFDIR)) {
		fprintf(stderr, "application data directory is actually not a directory.\n");
		abort();
	}
	if (access(path.c_str(), R_OK|W_OK|X_OK) != 0) {
		fprintf(stderr, "no write permission on application data directory.\n");
		abort();
	}
}

static std::string get_app_path() {
#ifdef WIN32
	std::string path=getenv("APPDATA");
	path+="/timetagger";
#else
	std::string path=getenv("HOME");
	path+="/.timetagger";
#endif
std::cerr << path << std::endl;
	ensure_directory(path);
	ensure_directory(path+"/data");
	ensure_directory(path+"/scripts");

	return path;
}


Datastore::Datastore(jsVar *config) {
	catalog=0;
	std::string app_path=get_app_path();
	catalog_path=app_path+"/catalog.json";
	data_path=app_path+"/data";
	std::ifstream in(catalog_path.c_str());
	if (in.good()) {
		in >> catalog;
		in.close();
	}

	if(!catalog) {
		std::stringstream ss;
		ss << JSON( "datasets"<< JSEMPTY
				 << "scripts" << JSEMPTY
				 << "configs" << JSEMPTY );
		ss.seekg(0);
		ss >> catalog;
	}
}

void Datastore::writeCatalog() {
	std::ofstream out(catalog_path.c_str());
	if (out.good()) {
		out << catalog;
		out.close();
	}
}

int Datastore::jsrpcGetDatasetList(JSON_FRAGMENT_STREAM &json, jsVar *param) {
	json << "result";

	JSON_STREAM &js=json.os;
	js << '[';
	bool first=true;
	jsVar *datasets=catalog->getValue("datasets");

	for (jsVar::iterator i=datasets->begin(); i!=datasets->end(); ++i) {
		if (!first) js << ',';
		first=false;

		jsVar *ds=i->second;
		std::string name;
		std::string description;
		std::string classname;
		long date=0;

		ds->getStr("name",name);
		ds->getStr("classname",classname);
		ds->getStr("description",description);
		ds->getNumber("date", date);
		js << JSON(
				"id" << i->first
			<<	"name" << name
			<<	"classname" << classname
			<<	"description" << description
			<<	"date" << date
		);

	}
	js << ']';
	return HTTP_OK;
}

std::string Datastore::get_datapath() {
	return get_app_path()+"/data";
}

std::string Datastore::get_dumppath() {
	return get_app_path()+"/dump";
}


int Datastore::jsrpcCreateDataset(JSON_FRAGMENT_STREAM &json, jsVar *param) {
	std::string name;
	std::string description;
	std::string classname;
	std::string comment;

	jsVar *datasets=catalog->getValue("datasets");

	std::string id=IdGenerator::get_id();

	if (param->getStr("name", name)) {
		jsVar *meta=param->clone();

		std::string path=data_path+"/"+id+".json";
		std::ofstream ds(path.c_str());
		if (ds.good()) {
			ds << meta;
			ds.close();
		}

		// meta.removeValue("data");
		// meta.removeValue("config");
		datasets->setValue(id, meta);

		writeCatalog();

		json << JSKEY("result") << JSEMPTY;
		return HTTP_OK;

	} else {

		json << "error" << JSON("code" << 200 << "message" << "missing dataset name");
		return HTTP_OK;
	}
}

int Datastore::jsrpcGetDataset(JSON_FRAGMENT_STREAM &json, jsVar *param) {
	std::string id;
	jsVar *datasets=catalog->getValue("datasets");
	if (param && param->getStr("id", id)) {
		jsVar *ds=datasets->getValue(id);
		if (ds) {
			jsVar *res=ds->clone();
			res->setValue("id", param->getValue("id")->clone());
			res->removeValue("data");
			json << "result" << res;
			delete res;

		} else {
			json << "error"
				<< JSON( 	"code" << 200
						<< 	"message" << "bad dataset id" );
		}
	} else {
		json << "error"
			<< JSON( 	"code" << 200
					<< 	"message" << "missing dataset id" );
	}
	return HTTP_OK;
}

/*static void jsonDownsample2(std::ostream &out, jsVar *data, int maxpoints, bool normalize=true) {
	int counts;
	long long duration;
	int overflows;
	int npoints;
	int tmin;
	int tmax;

	data->getNumber("duration", duration);
	data->getNumber("counts", counts);
	data->getNumber("overflows", overflows);
	data->getNumber("tmin", tmin);
	data->getNumber("tmax", tmax);

	out << '{';

	out << JSKEY("duration") << duration << ',';
	out << JSKEY("counts") << counts << ',';
	out << JSKEY("overflows") << overflows << ',';
	out << JSKEY("tmin") << tmin << ',';
	out << JSKEY("tmax") << tmax << ',';

	data->getNumber("points", npoints);
	jsVar *points=data->getValue("data");

	int seq_count=points->length();
	double *values=new double [ seq_count * npoints * 2];

	double *p = values;
	for (int n=0; n<seq_count; ++n) {
		jsVar *series=points->getValue(n);
		int np=series->length();
		for (int m=0; m<np; ++m) {
			jsVar *point=series->getValue(m);
			double t=0, v=0;
			if (point) {
				point->getValue(0)->getNumber(t);
				point->getValue(1)->getNumber(v);
			}
			*p++=t;
			*p++=v;
		}
	}
	values=DiagramData::downsample2(values, seq_count, npoints, maxpoints);
	if (0 &&normalize) {
		double vmax=INT_MAX;
		for (int n=0; n<seq_count; ++n) {
			for (int m=0; m<npoints; ++m) {
				double v=values[n*npoints*2 + m*2 + 1];
				vmax=std::min(vmax, v);
			}
		}
		for (int n=0; n<seq_count; ++n) {
			for (int m=0; m<npoints; ++m) {
				values[n*npoints*2 + m*2 + 1] /= vmax;
			}
		}
	}
	out << JSKEY("data") << '[';
	for (int n=0; n<seq_count; ++n) {
		if (n>0) out << ',';
		out << '[';
		for (int m=0; m<npoints; ++m) {
			if (m>0) out << ',';
			double t=values[n*npoints*2 + m*2];
			double v=values[n*npoints*2 + m*2 + 1];
			out << '[' << t << ',' << v << ']';
		}
		out << ']';
	}
	out << ']';

	out << '}';

	delete [] values;
}


static void jsonDownsample(std::ostream &out, jsVar *data, int maxpoints, bool normalize=true) {
	int counts;
	long long duration;
	int overflows;
	int npoints;
	int tmin;
	int tmax;

	data->getNumber("duration", duration);
	data->getNumber("counts", counts);
	data->getNumber("overflows", overflows);
	data->getNumber("tmin", tmin);
	data->getNumber("tmax", tmax);

	out << '{';

	out << JSKEY("duration") << duration << ',';
	out << JSKEY("counts") << counts << ',';
	out << JSKEY("overflows") << overflows << ',';
	out << JSKEY("tmin") << tmin << ',';
	out << JSKEY("tmax") << tmax << ',';

	data->getNumber("points", npoints);
	jsVar *points=data->getValue("data");

	double *values=new double [ npoints * 2];

	double *p = values;
	for (int m=0; m<npoints; ++m) {
		jsVar *point=points->getValue(m);
		double t, v;
		point->getValue(0)->getNumber(t);
		point->getValue(1)->getNumber(v);
		*p++=t;
		*p++=v;
	}

	values=DiagramData::downsample(values, npoints, maxpoints);
	if (normalize) {
		double vmax=INT_MAX;
		for (int m=0; m<npoints; ++m) {
			double v=values[m*2 + 1];
			vmax=std::min(vmax, v);
		}
		for (int m=0; m<npoints; ++m) {
			values[m*2 + 1] /= vmax;
		}
	}
	out << '[';
	for (int m=0; m<npoints; ++m) {
		if (m>0) out << ',';
		double t=values[m*2];
		double v=values[m*2 + 1];
		out << '[' << t << ',' << v << ']';
	}
	out << ']';

	out << '}';

	delete [] values;
}
*/

int Datastore::jsrpcDownloadDataset(JSON_FRAGMENT_STREAM &json, jsVar *param) {
	std::string id;
	jsVar *datasets=catalog->getValue("datasets");
	if (param && param->getStr("id", id)) {
		jsVar *ds=datasets->getValue(id);
		if (ds) {
			jsVar *res=ds->clone();
			res->setValue("id", param->getValue("id")->clone());

			int maxpoints=1000;
			param->getNumber("maxpoints", maxpoints);
			if (maxpoints==0) {
				json << "result" << res;
			} else {
				json << "result" << JSON(
								"id" << res->getValue("id")
							<<	"name" << res->getValue("name")
							<<	"description" << res->getValue("description")
							<<	"comment" << res->getValue("comment")
							<< 	"classname" << res->getValue("classname")
							<< 	"date" << res->getValue("date")
							<< 	"friendly_date" << res->getValue("friendly_date")
							<< 	"config" << res->getValue("config")
							<< 	"descriptor" << res->getValue("descriptor")
							<< 	"data"<< res->getValue("data")

						);
			}
			delete res;
		} else {
			json << "error"
				<< JSON( 	"code" << 200
						<< 	"message" << "bad dataset id" );
		}
	} else {
		json << "error"
			<< JSON( 	"code" << 200
					<< 	"message" << "missing dataset id" );
	}
	return HTTP_OK;
}

int Datastore::jsrpcRemoveDataset(JSON_FRAGMENT_STREAM &json, jsVar *param) {
	std::string id;
	jsVar *datasets=catalog->getValue("datasets");
	if (param && param->getStr("id", id)) {
		jsVar *ds=datasets->getValue(id);
		if (ds) {
			datasets->removeValue(id);
			writeCatalog();

			std::string path=data_path+"/"+id+".json";
			unlink(path.c_str());

			json << "result" << JSEMPTY;
		} else {
			json << "error"
				<< JSON( 	"code" << 200
						<< 	"message" << "bad dataset id" );
		}
	} else {
		json << "error"
			<< JSON( 	"code" << 200
					<< 	"message" << "missing dataset id" );
	}
	return HTTP_OK;
}

int Datastore::jsrpcSaveDataset(JSON_FRAGMENT_STREAM &json, jsVar *param) {
	std::string id;
	jsVar *datasets=catalog->getValue("datasets");
	if (param && param->getStr("id", id)) {
		jsVar *ds=datasets->getValue(id);
		if (ds) {

			std::string name;
			std::string description;
			std::string comment;

			if (param->getStr("name", name)) ds->setValue("name", name);
			if (param->getStr("description", name)) ds->setValue("description", name);
			if (param->getStr("comment", name)) ds->setValue("comment", name);

			writeCatalog();
			json << "result" << JSEMPTY;
		} else {
			json << "error"
				<< JSON( 	"code" << 200
						<< 	"message" << "bad dataset id" );
		}
	} else {
		json << "error"
			<< JSON( 	"code" << 200
					<< 	"message" << "missing dataset id" );
	}
	return HTTP_OK;
}


void Datastore::jsonWriteConfig(std::ostream &out) {
	out << "{}";

}


