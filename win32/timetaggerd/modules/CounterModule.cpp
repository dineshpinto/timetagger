#include "CounterModule.h"

#include "../../backend/Logger.h"
#include "../DiagramData.h"
#include "../minmax.h"

// Counter.cpp is already included in BackendControl.h
// #include "../../backend/Counter.cpp"

INSTALL_LOG_FACILITY(Logger::APPLICATION)


std::string CounterModule::Factory::classname() {
	return "counter";
}

CounterModule::Factory CounterModule::_factory;

ModuleController *CounterModule::Factory::create(TimeTagger *tagger, jsVar *parameter) {
	return new CounterModule(tagger, parameter);
}

int CounterModule::Factory::jsonWriteDescriptor(JSON_STREAM &json) {
	json << "[";
	json << JSON("name" << "channel" 	<< "type" << "channel-set" << "default" << 0 );
	json << ",";
	json << JSON("name" << "binwidth"	<< "type" << "timestamp" << "default" << 1000000000 );
	json << ",";
	json << JSON("name" << "bins"		<< "type" << "int" << "default" << 100 );
	json << "]";
	return HTTP_OK;
}


CounterModule::CounterModule(TimeTagger* tagger, channel_t _chan, timestamp_t _binwidth, int _bins)
	: ModuleController(tagger), channel(_chan), binwidth(_binwidth), bins(_bins) {

	std::vector<channel_t> chan_vect;
	unsigned mask=1;
	for (int n=0; n<8; ++n) {
		if (channel&mask)
			chan_vect.push_back(n);
	}

	this->counter=new Counter(tagger, chan_vect, binwidth, bins);
}

CounterModule::CounterModule(TimeTagger* tagger, jsVar *parameter)
	: ModuleController(tagger) {
	long long binwidth=1000;
	int bins=1;
	unsigned channels=0;

	parameter->getNumber("channel", channels);
	parameter->getNumber("binwidth", binwidth);
	parameter->getNumber("bins", bins);

	std::vector<channel_t> chan_vect;
	unsigned mask=1;
	for (int n=0; n<8; ++n) {
		if (channels&mask)
			chan_vect.push_back(n);
	}

	set_common_config(parameter);
	this->counter=new Counter(tagger, chan_vect, binwidth, bins);
}


CounterModule::~CounterModule() {
}

std::string CounterModule::classname() {
	return _factory.classname();
}

void CounterModule::jsonWriteDescriptor(JSON_STREAM &out) {
	_factory.jsonWriteDescriptor(out);
}

int CounterModule::jsrpcGetDescriptor(JSON_FRAGMENT_STREAM &json, jsVar *parm) {
	json << "result";
	jsonWriteDescriptor(json.os);
	return HTTP_OK;
}

int CounterModule::jsrpcGetConfig(JSON_FRAGMENT_STREAM &json, jsVar *) {
	json << "result";
	jsonWriteConfig(json.os);
	return HTTP_OK;
}

void CounterModule::jsonWriteConfig(JSON_STREAM &out) {
	out << JSON("name" << name
			<<	"id" << get_id()
			<<	"classname" << classname()
			<<	"state" << status_str()
			<<	"channel" << (int)channel
			<<	"binwidth" << binwidth
			<< 	"bins" << bins
		);
}

int CounterModule::jsrpcSetConfig(JSON_FRAGMENT_STREAM &out, jsVar *parameter) {
	//parameter->getNumber("channel", channel);
	parameter->getNumber("binwidth", binwidth);
	parameter->getNumber("bins", bins);
	set_common_config(parameter);

	int state=counter->status();
	delete counter;
	std::vector<channel_t> chans;
	counter=new Counter(get_tagger(), chans, binwidth, bins);

	if (state==STATE_STOPPED) {
		counter->stop();
		counter->clear();
	}

	out << JSKEY("result")
		<< "{}";
	return 200;
}

void CounterModule::jsrpcGetMetaData(JSON_FRAGMENT_STREAM &json, std::vector<int> &data) {

	int vmin, vmax;
	get_min_max(data, vmin, vmax);
	json 	<< "tmin" << 0
			<< "step" << binwidth
			<< "tmax" << (binwidth*bins)
			<< "overflows" << getOverflows()
			<< "duration"  << getDuration()
			<< "vmin" << vmin
			<< "vmax" << vmax;
}

int CounterModule::jsrpcGetData(JSON_FRAGMENT_STREAM &json, jsVar *param) {

	int threshold=0;
	std::vector<int> data;
	counter->getData([&](int d1, int d2) {
		data.resize(d1*d2);
		return data.data();
	});
	if (threshold) {
		DiagramData::downsample(data, threshold);
	}

	JSON_FRAGMENT_STREAM result =
			(json << "result" << JSON_FRAGMENT( "downsampled" << threshold ) );

	jsrpcGetMetaData(result, data);
	result << "data" << data << JS_ENDOBJECT;

	return HTTP_OK;
}
