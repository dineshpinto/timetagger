#include "CountbetweenmarkersModule.h"

#include "../../backend/Logger.h"
#include "../DiagramData.h"
#include "../minmax.h"

// Counter.cpp is already included in BackendControl.h
// #include "../../backend/Counter.cpp"

INSTALL_LOG_FACILITY(Logger::APPLICATION)


std::string CountbetweenmarkersModule::Factory::classname() {
	return "countbetweenmarkers";
}

CountbetweenmarkersModule::Factory CountbetweenmarkersModule::_factory;

ModuleController *CountbetweenmarkersModule::Factory::create(TimeTagger *tagger, jsVar *parameter) {
	return new CountbetweenmarkersModule(tagger, parameter);
}

int CountbetweenmarkersModule::Factory::jsonWriteDescriptor(JSON_STREAM &json) {
	json << "[";
	json << JSON("name" << "bins" 	<< "type" << "int" << "default" << 1 );
	json << ",";
	json << JSON("name" << "click_channel"		<< "type" << "channel" << "default" << 0 );
	json << ",";
	json << JSON("name" << "marker_channel"		<< "type" << "channel" << "default" << 7 );
	json << ",";
	json << JSON("name" << "stop_channel"		<< "type" << "channel" << "default" << 3 );
	json << "]";
	return HTTP_OK;
}


CountbetweenmarkersModule::CountbetweenmarkersModule(TimeTagger* tagger, int _bins,
													 channel_t _click_channel, channel_t _marker_channel, channel_t _stop_channel )
	: ModuleController(tagger), bins(_bins), click_channel(_click_channel), marker_channel(_marker_channel), stop_channel(_stop_channel) {


	this->counter=new CountBetweenMarkers(tagger, bins, click_channel, marker_channel, stop_channel);
}

CountbetweenmarkersModule::CountbetweenmarkersModule(TimeTagger* tagger, jsVar *parameter)
	: ModuleController(tagger) {

	parameter->getNumber("bins", bins);
	parameter->getNumber("click_channel", click_channel);
	parameter->getNumber("marker_channel", marker_channel);
	parameter->getNumber("stop_channel", stop_channel);

	set_common_config(parameter);
	this->counter=new CountBetweenMarkers(tagger, bins, click_channel, marker_channel, stop_channel);
}


CountbetweenmarkersModule::~CountbetweenmarkersModule() {
}

std::string CountbetweenmarkersModule::classname() {
	return _factory.classname();
}

void CountbetweenmarkersModule::jsonWriteDescriptor(JSON_STREAM &out) {
	_factory.jsonWriteDescriptor(out);
}

int CountbetweenmarkersModule::jsrpcGetDescriptor(JSON_FRAGMENT_STREAM &json, jsVar *parm) {
	json << "result";
	jsonWriteDescriptor(json.os);
	return HTTP_OK;
}

int CountbetweenmarkersModule::jsrpcGetConfig(JSON_FRAGMENT_STREAM &json, jsVar *) {
	json << "result";
	jsonWriteConfig(json.os);
	return HTTP_OK;
}

void CountbetweenmarkersModule::jsonWriteConfig(JSON_STREAM &out) {
	out << JSON("name" << name
			<<	"id" << get_id()
			<<	"classname" << classname()
			<<	"state" << status_str()
			<< 	"bins" << bins
			<< 	"click_channel" << click_channel
			<< 	"marker_channel" << marker_channel
			<< 	"stop_channel" << stop_channel
		);
}

int CountbetweenmarkersModule::jsrpcSetConfig(JSON_FRAGMENT_STREAM &out, jsVar *parameter) {
	parameter->getNumber("bins", bins);
	parameter->getNumber("click_channel", click_channel);
	parameter->getNumber("marker_channel", marker_channel);
	parameter->getNumber("stop_channel", stop_channel);

	set_common_config(parameter);

	int state=counter->status();
	delete counter;
	this->counter=new CountBetweenMarkers(this->get_tagger(), bins, click_channel, marker_channel, stop_channel);

	if (state==STATE_STOPPED) {
		counter->stop();
		counter->clear();
	}

	out << JSKEY("result")
		<< "{}";
	return 200;
}

void CountbetweenmarkersModule::jsrpcGetMetaData(JSON_FRAGMENT_STREAM &json, std::vector< int > &data) {

	int vmin, vmax;
	get_min_max(data, vmin, vmax);
	json 	<< "tmin" << 0
			<< "overflows" << getOverflows()
			<< "duration"  << getDuration()
			<< "vmin" << vmin
			<< "vmax" << vmax;
}

int CountbetweenmarkersModule::jsrpcGetData(JSON_FRAGMENT_STREAM &json, jsVar *param) {

	int threshold=0;
	std::vector<int> data;
	counter->getData([&](int d) {data.resize(d); return data.data();});
	if (threshold) {
		DiagramData::downsample(data, threshold);
	}

	JSON_FRAGMENT_STREAM result =
			(json << "result" << JSON_FRAGMENT( "downsampled" << threshold ) );

	jsrpcGetMetaData(result, data);
	result << "data" << data[0] << JS_ENDOBJECT;

	return HTTP_OK;

}




