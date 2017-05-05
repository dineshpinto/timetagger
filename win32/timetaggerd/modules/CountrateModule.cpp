#include "CountrateModule.h"

#include "../DiagramData.h"
#include "../minmax.h"

#include <stdio.h>
#include <string>
#include <stdlib.h>
#include <iostream>

#include "../../backend/Logger.h"



INSTALL_LOG_FACILITY(Logger::APPLICATION)


std::string CountrateModule::Factory::classname() {
	return "countrate";
}

CountrateModule::Factory CountrateModule::_factory;

ModuleController *CountrateModule::Factory::create(TimeTagger *tagger, jsVar *parameter) {
	return new CountrateModule(tagger, parameter);
}

int CountrateModule::Factory::jsonWriteDescriptor(JSON_STREAM &json) {
	json << "[";
	json << JSON("name" << "channel" 	<< "type" << "channel-set" << "default" << 0 );
	json << "]";
	return HTTP_OK;
}


CountrateModule::CountrateModule(TimeTagger* tagger, channel_t _chan)
	: ModuleController(tagger), channel(_chan) {

	std::vector<channel_t> chan_vect;
	unsigned mask=1;
	for (int n=0; n<8; ++n) {
		if (channel&mask)
			chan_vect.push_back(n);
		mask = mask <<1;
	}

	this->countrate=new Countrate(tagger, chan_vect);
}

CountrateModule::CountrateModule(TimeTagger* tagger, jsVar *parameter)
	: ModuleController(tagger), channel(0) {
	parameter->getNumber("channel", channel);

	std::vector<channel_t> chan_vect;
	unsigned mask=1;
	for (int n=0; n<8; ++n) {
		if (channel&mask)
			chan_vect.push_back(n);
		mask = mask << 1;
	}
	set_common_config(parameter);

	this->countrate=new Countrate(tagger, chan_vect);
}


CountrateModule::~CountrateModule() {
}

std::string CountrateModule::classname() {
	return _factory.classname();
}

void CountrateModule::jsonWriteDescriptor(JSON_STREAM &out) {
	_factory.jsonWriteDescriptor(out);
}

int CountrateModule::jsrpcGetDescriptor(JSON_FRAGMENT_STREAM &json, jsVar *parm) {
	json << "result";
	jsonWriteDescriptor(json.os);
	return HTTP_OK;
}

int CountrateModule::jsrpcGetConfig(JSON_FRAGMENT_STREAM &json, jsVar *) {
	json << "result";
	jsonWriteConfig(json.os);
	return HTTP_OK;
}

void CountrateModule::jsonWriteConfig(JSON_STREAM &out) {
	out << JSON("name" << name
			<<	"id" << get_id()
			<<	"classname" << classname()
			<<	"state" << status_str()
			<<	"channel" << (int)channel
		);
}

int CountrateModule::jsrpcSetConfig(JSON_FRAGMENT_STREAM &out, jsVar *parameter) {
	set_common_config(parameter);

	parameter->getNumber("channel", channel);

	std::vector<channel_t> chan_vect;
	unsigned mask=1;
	for (int n=0; n<8; ++n) {
		if (channel&mask)
			chan_vect.push_back(n);
		mask = mask << 1;
	}

	int state=countrate->status();
	delete countrate;
	countrate=new Countrate(get_tagger(), chan_vect);
	if (state==STATE_STOPPED) {
		countrate->stop();
		countrate->clear();
	}

	out << "result" << JSEMPTY;
	return HTTP_OK;
}


void CountrateModule::jsrpcGetMetaData(JSON_FRAGMENT_STREAM &json, std::vector< double > &data) {

	double vmin, vmax;
	get_min_max(data, vmin, vmax);
	json
			<< "overflows" << getOverflows()
			<< "duration"  << getDuration()
			<< "vmin" << vmin
			<< "vmax" << vmax;
}

int CountrateModule::jsrpcGetData(JSON_FRAGMENT_STREAM &json, jsVar *param) {
	int threshold=0;
	std::vector<double> data;
	countrate->getData([&](int d) {data.resize(d); return data.data();});
	if (threshold) {
		DiagramData::downsample(data, threshold);
	}

	JSON_FRAGMENT_STREAM result =
			(json << "result" << JSON_FRAGMENT( "downsampled" << threshold ) );

	jsrpcGetMetaData(result, data);
	result << "data" << data << JS_ENDOBJECT;

	return HTTP_OK;

}




