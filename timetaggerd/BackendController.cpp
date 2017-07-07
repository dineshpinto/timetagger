/*
 * BackendControl.cpp
 *
 *  Created on: 23.04.2013
 *      Author: mag
 */

#include "ServerController.h"
#include "BackendController.h"


#include "modules/CountrateModule.h"
#include "modules/CounterModule.h"
#include "modules/CountbetweenmarkersModule.h"
#include "DiagramData.h"

#include "../backend/Logger.h"

#include <sstream>
#include <stdlib.h>
#include <algorithm>
#include <climits>

#include "minmax.h"

INSTALL_LOG_FACILITY(Logger::APPLICATION)

BackendController::BackendController(const std::string &serial) {
	init(serial);
}

BackendController::~BackendController() {
	freeTimeTagger(_tagger);
}

BackendController::BackendController(const std::string &serial, jsVar *config) {
	init(serial);


	std::string name;
	int max_id;
	if (config->getStr("name", name))
		this->_name=name;
	if (config->getNumber("max_id", max_id))
		IdGenerator::set_max(max_id);

	debug("loading modules");
	jsVar *modules=config->getValue("modules");
	if (modules) {
		for (jsVar::iterator i=modules->begin(); i!=modules->end(); ++i) {
			std::string id=i->first;
			jsVar *cfg=i->second;
			ModuleController *bc=newModule(cfg);
			// shall be stopped on startup
			if (bc) {
				bc->stop();
				bc->clear();

				bc->id=i->first;
				if (bc) slots[i->first]=bc;
			}
		}
	}

	jsVar *dac=config->getValue("dac");
	if (dac && dac->isArray()) {
		size_t sz=dac->length();
		for (size_t n=0; n<sz; ++n) {
			double voltage;
			if (dac->getNumber(n, voltage)) {
				_tagger->setTriggerLevel(n, voltage);
			}
		}
	}
	bool filter;
	if (config->getBool("filter", filter))
		_tagger->setFilter(filter);

	debug("backend startup done");
}

void BackendController::init(std::string serial) {
	info("starting backend for device %s", serial.c_str());
	_tagger=createTimeTagger(serial);

	registerModule(&CountrateModule::_factory);
	registerModule(&CounterModule::_factory);
	registerModule(&CountbetweenmarkersModule::_factory);
}

void BackendController::registerModule(ModuleController::AbstractFactory *factory ) {
	module_factories[factory->classname()]=factory;
}

int BackendController::jsonGetInfo(JSON_STREAM &js) {
	js << JSON(
			 "id" << get_serial()
		<<	"serial" << get_serial()
		<<	"name" << get_name()
		<<	"hardware" << get_hardware_name()
		<<	"state" << get_status_tag()
		<<	"module_count" << get_module_count()
		<<  "overflows" << get_overflows()
	);
	return 200;
}


void BackendController::jsonWriteConfig(JSON_STREAM &json) {
	json << JSON(
			"name" << this->_name
		<< 	"max_id" << IdGenerator::get_max()
		<< "dac" << get_tagger()->getTriggerLevel()
		<< "filter" << get_tagger()->getFilter()
		<< "modules");

	//bool first=true;

	//TODO
	/*
	for (std::map<std::string, BackendModule *>::iterator i=slots.begin(); i!=slots.end(); ++i) {
		if (!first) {
			i->second->jsonWriteConfig(vfirst << i->first << ObjectWriter());
		} else {
			i->second->jsonWriteConfig(vnext << i->first << ObjectWriter());
		}
		first=false;
	}
	vfirst << ObjectWriter::endObject();
	 */
}


int BackendController::jsrpcGetInfo(JSON_FRAGMENT_STREAM &json, jsVar *params) {
	json << "result" ;
	this->jsonGetInfo(json.os);
	return 200;
}

int BackendController::jsrpcGetDACSettings(JSON_FRAGMENT_STREAM &json, jsVar *) {
	json << "result" << get_tagger()->getTriggerLevel();
	return 200;
}

int BackendController::jsrpcSetDACValue(JSON_FRAGMENT_STREAM &json, jsVar *params) {
	int channel;
	double value;
	if (params->getNumber("channel", channel) && params->getNumber("value", value)) {
		get_tagger()->setTriggerLevel(channel, value);
		json << "result" << JSON( "message" << "dac value set" );
		return HTTP_OK;
	}
	json << "error" << JSON( "code" << ERR_INVALID_PARAMS << "message" << MSG_INVALID_PARAMS << "data" << "no or bad channel given");
	return HTTP_OK;
}

int BackendController::jsrpcGetFilter(JSON_FRAGMENT_STREAM &json, jsVar *params) {
	json << "result" << JSON( "filter" << get_tagger()->getFilter() );
	return 200;

}

int BackendController::jsrpcSetFilter(JSON_FRAGMENT_STREAM &json, jsVar *params) {
	bool value;
	if (params->getBool("filter", value)) {
		get_tagger()->setFilter(value);
		json << "result" << JSON( "message" << "filter set" );
		return HTTP_OK;
	}
	json << "error" << JSON( "code" << ERR_INVALID_PARAMS << "message" << MSG_INVALID_PARAMS << "data" << "no or bad filter value given");
	return HTTP_OK;
}

int BackendController::jsrpcGetTestsignal(JSON_FRAGMENT_STREAM &json, jsVar *params) {
	std::vector<bool> enable(8);
	for(int i=0; i<8; i++) enable[i] = get_tagger()->getTestsignal(i);
	json << "result" << JSON( "testsignal" << enable );
	return HTTP_OK;
}

int BackendController::jsrpcSetTestsignal(JSON_FRAGMENT_STREAM &json, jsVar *params) {
	int channel;
	bool enable;
	if (params->getNumber("channel", channel) && params->getBool("enable", enable)) {
		get_tagger()->setTestsignal(channel, enable);
		json << "result" << JSON( "message" << "Testsignal set" );
		return HTTP_OK;
	}
	json << "error" << JSON( "code" << ERR_INVALID_PARAMS << "message" << MSG_INVALID_PARAMS << "data" << "no or bad channel value given");
	return HTTP_OK;
}

int BackendController::jsrpcGetLineDelay(JSON_FRAGMENT_STREAM &json, jsVar *params) {
	json << "result" <<  get_tagger()->getLineDelay();
	return 200;
}

int BackendController::jsrpcSetLineDelay(JSON_FRAGMENT_STREAM &out, jsVar *params) {
	int channel;
	long long value;
	if (params->getNumber("channel", channel))
		if (params->getNumber("delay", value)) {
			get_tagger()->setLineDelay(channel, value);
			out << "result" << JSON( "message" << "line delay set" );
			return HTTP_OK;
		}
	out << "error" << JSON( "code" << ERR_INVALID_PARAMS << "message" << MSG_INVALID_PARAMS << "data" << "channel or delay value bad or missing");
	return HTTP_OK;
}

int BackendController::jsrpcRename(JSON_FRAGMENT_STREAM &json, jsVar *params) {

	std::string n;
	if (params->getStr("name",n)) {
		this->_name=n;
		json << "result" << JSON( "message" << "device renamed" );
		return HTTP_OK;
	}
	json << "error" << JSON( "code" << ERR_INVALID_PARAMS << "message" << MSG_INVALID_PARAMS << "data" << "new name bad or missing");
	return HTTP_OK;
}

int BackendController::jsonWriteClasses(JSON_STREAM &json) {
	bool first=true;
	json << '[';
	for (std::map<std::string, ModuleController::AbstractFactory *>::iterator i=module_factories.begin();
			i!=module_factories.end(); ++i) {
		if(!first) json << ',';
		first=false;
		json << "{ \"name\": \"" << i->first << "\", \"parameter\":" ;
		i->second ->jsonWriteDescriptor(json);
		json  << "}";
	}
	json  << "]";

	return HTTP_OK;
}

int BackendController::jsrpcGetClasses(JSON_FRAGMENT_STREAM &json, jsVar *params) {
	json << "result";
	jsonWriteClasses(json.os);
	return HTTP_OK;
}

int BackendController::jsrpcGetModules(JSON_STREAM &json) {
	bool first=true;
	json << '[';
	for (std::map<std::string, ModuleController *>::iterator i=slots.begin(); i!=slots.end(); ++i) {
		if (!first) json << ',';
		first=false;
		ModuleController *m=i->second;
		if (m) {
			m->jsonGetStatus(json);
		}
	}

	json << "]";	return HTTP_OK;
}

int BackendController::jsrpcGetModules(JSON_FRAGMENT_STREAM &json, jsVar *params) {
	json << "result";
	jsrpcGetModules(json.os);
	return HTTP_OK;
}

int BackendController::jsrpcStartModule(JSON_FRAGMENT_STREAM &json, jsVar *params) {
	ModuleController *module=get_module(json, params);
	if (module) {
		module->start();
	}

	json << "result" << JSEMPTY;
	return HTTP_OK;
}

int BackendController::jsrpcStopModule(JSON_FRAGMENT_STREAM &out, jsVar *params) {
	ModuleController *module=get_module(out, params);
	if (module) {
		module->stop();
		out << "result" << JSEMPTY;
	}
	return HTTP_OK;
}

int BackendController::jsrpcClearModule(JSON_FRAGMENT_STREAM &out, jsVar *params) {
	ModuleController *module=get_module(out, params);
	if (module) {
		module->clear();
		out << "result" << JSEMPTY;
	}
	return HTTP_OK;
}

int BackendController::jsrpcRemoveModule(JSON_FRAGMENT_STREAM &json, jsVar *params) {
	ModuleController *module=get_module(json, params);
	if (module) {
		remove_module(module->id);
		json << "result" << JSEMPTY;
	}
	return HTTP_OK;
}

int BackendController::jsrpcCreateModule(JSON_FRAGMENT_STREAM &json, jsVar *params) {
	std::string classname;
	if (params->getStr("classname", classname)) {
		ModuleController *bc=newModule(params);
		if (bc) {
			add_slot(bc);
			json << "result" << JSON( "message" << "module created" << "name" << bc->name << "id" << bc->id);
			return HTTP_OK;
		} else {
			json << "error" << JSON(
									"code" << ERR_BAD_MODULE_CLASS
								<< 	"message" << MSG_BAD_MODULE_CLASS
								);
			return HTTP_OK;
		}
	} else {
		json << "error" << JSON(
								"code" << ERR_INVALID_PARAMS
							<< 	"message" << ERR_INVALID_PARAMS
							<<  "data" << "missing classname"
				);
		return HTTP_OK;
	}
}

ModuleController *BackendController::newModule(jsVar *params) {
	std::string classname;
	if (params->getStr("classname", classname)) {
		info("creating '%s' module", classname.c_str());
		std::map<std::string, ModuleController::AbstractFactory *>::iterator
			i=this->module_factories.find(classname);
		if (i!=this->module_factories.end()) {
			ModuleController::AbstractFactory *factory=i->second;
			return factory->create(get_tagger(), params);

		} else
			error("creating '%s' module failed: no such module class", classname.c_str());
	}
	return 0;
}

int BackendController::jsrpcGetModuleDescriptor(JSON_FRAGMENT_STREAM &json, jsVar *params) {
	ModuleController *module=get_module(json,params);
	if (module) {
		module->jsrpcGetDescriptor(json, params);
	}
	return HTTP_OK;
}

int BackendController::jsrpcGetModuleConfig(JSON_FRAGMENT_STREAM &json, jsVar *params) {
	ModuleController *module=get_module(json, params);
	if (module) {
		module->jsrpcGetConfig(json, params);
	}
	return 200;
}

int BackendController::jsrpcSetModuleConfig(JSON_FRAGMENT_STREAM &json, jsVar *params) {
	ModuleController *module=get_module(json, params);
	if (module) {
		module->jsrpcSetConfig(json, params);
	}
	return 200;
}

int BackendController::jsrpcGetData(JSON_FRAGMENT_STREAM &json, jsVar *params) {
	ModuleController *module=get_module(json, params);
	if (module) {
		module->jsrpcGetData(json, params);
	}
	return 200;
}



std::string BackendController::get_serial() {
	return _tagger->getSerial();
}

std::string BackendController::get_name() {
	return _name;
}


std::string BackendController::get_hardware_name() {
	int model=_tagger->getBoardModel();
	if (model<0) return "unknown";
	switch(model) {
	case 0: return "Unknown";
	case 1: return "XEM3001v1";
	case 2: return "XEM3001v2";
	case 3: return "XEM3010";
	case 4: return "XEM3005";
	case 5: return "XEM3001CL";
	case 6: return "XEM3020";
	case 7: return "XEM3050";
	case 8: return "XEM9002";
	case 9: return "XEM3001RB";
	case 10: return "XEM5010";
	}
	return "unknown";
}

std::string BackendController::get_status_tag() {
	int status=_tagger->getStatus();
	switch(status) {
	case STATE_ERROR: return "error";
	case STATE_IDLE: return "idle";
	case STATE_RUNNING: return "running";
	case STATE_STOPPED: return "stopped";
	default: return "error";
	}
}


int BackendController::get_module_count() {
	return this->slots.size();
}

TimeTagger *BackendController::get_tagger() {
	return _tagger;
}

long long BackendController::get_overflows() {
	return _tagger ? 0 : -1; //TODO
}

std::string BackendController::add_slot(ModuleController *module) {
	std::string id=IdGenerator::get_id();

	slots[id]=module;
	module->id=id;
	return id;
}

ModuleController *BackendController::get_module(std::string id) {
	std::map<std::string, ModuleController *>::iterator i=slots.find(id);
	if (i!=slots.end())
		return i->second;
	return 0;
}

ModuleController *BackendController::get_module(JSON_FRAGMENT_STREAM &json, jsVar *params) {
	std::string id;
	if (params && params->getStr("module", id)) {
		ModuleController *bc=get_module(id);
		if (!bc)
			json << "error" << JSON( "code" << ERR_BAD_MODULE_ID << "message" << MSG_BAD_MODULE_ID << "data" << "module does not exist");
		return bc;
	}
	json << "error" << JSON(
							"code" << ERR_INVALID_PARAMS
						<< 	"message" << ERR_INVALID_PARAMS
						<<  "data" << "missing module id"
			);

	return 0;
}

void BackendController::remove_module(const std::string &id) {
	std::map<std::string, ModuleController *>::iterator i=slots.find(id);
	if (i==slots.end())
		return;
	ModuleController *module=i->second;
	slots.erase(id);
	module->stop();
	delete module;
}

/* --- */

std::string ModuleController::status_str() {
	switch(status()) {
	case STATE_IDLE: return "idle";
	case STATE_RUNNING: return "running";
	case STATE_STOPPED: return "stopped";
	default: return "error";
	}
}

void ModuleController::start() {
	_Iterator *iter=get_iterator();
	if (iter) iter->start();
}

void ModuleController::stop() {
	_Iterator *iter=get_iterator();
	if (iter) iter->stop();
}

void ModuleController::clear() {
	_Iterator *iter=get_iterator();
	if (iter) iter->clear();
}

int ModuleController::status() {
	_Iterator *iter=get_iterator();
	if (iter) return iter->status();
	else return STATE_STOPPED;
}

long ModuleController::getOverflows() {
	return this->get_iterator()->getOverflows();
}

timestamp_t ModuleController::getDuration() {
	return this->get_iterator()->getDuration();
}


int ModuleController::jsonGetStatus(JSON_STREAM &json) {
	json << JSON(
				"id" << id
			<<	"class" << classname()
			<<	"name" << name
			<<	"state" << status_str()
	);
	return HTTP_OK;
}

void ModuleController::set_common_config(jsVar *parameter) {
	std::string sval;
	for (jsVar::iterator i = parameter->begin(); i!=parameter->end(); ++i) {
		std::string key=i->first;
		if (key=="name") {
			if (i->second->getStr(sval))
				name=sval;
		}
	}
}


/* --- */

/* ---- */
#if 0

BackendController::_Countrate::_Countrate(TimeTagger* tagger, int _chan)
: BackendModule(tagger),
  channel(_chan) {
	countrate=new Countrate(tagger, channel);

}

std::string BackendController::_Countrate::class_name() {
	return "countrate";
}

void BackendController::_Countrate::start() {
	if (countrate) countrate->start();
}

void BackendController::_Countrate::stop() {
	if (countrate) countrate->stop();
}

void BackendController::_Countrate::clear() {
	if (countrate) countrate->clear();
}

int BackendController::_Countrate::status() {
	if (countrate) return countrate->status();
	else return STATE_STOPPED;
}

int BackendController::_Countrate::jsonGetDescriptor(JSON_FRAGMENT_STREAM &out, jsVar*) {
	out << "[";
	out << JSON("name" << "channel" 	<< "type" << "channel" << "default" << 0 );
	out << "]";
	return 200;
}

int BackendController::_Countrate::jsrpcGetDescriptor(JSON_FRAGMENT_STREAM &out, jsVar *parm) {
	out << JSKEY("result");
	jsonGetDescriptor(out, parm);
	return 200;
}

int BackendController::_Countrate::jsrpcGetConfig(JSON_FRAGMENT_STREAM &out, jsVar *) {
	out << JSKEY("result");
	jsonWriteConfig(out);
	return 200;
}

void BackendController::_Countrate::jsonWriteConfig(JSON_FRAGMENT_STREAM &out) {
	out << JSON("name" << name
			<<	"id" << id
			<< "classname" << "countrate"
			<<	"state" << status_str()
			<< "channel" << channel
			<< "state" << status_str()
		);
}

int BackendController::_Countrate::jsrpcSetConfig(JSON_FRAGMENT_STREAM &out, jsVar *parameter) {

	set_common_config(parameter);

	parameter->getNumber("channel", channel);
	std::vector<channel_t> chans;
	unsigned mask=1;
	for (int n=0; n<8; ++n) {
		if (channel&mask)
			chans.push_back(n);
		mask = mask << 1;
	}

	int state=countrate->status();
	delete countrate;
	countrate=new Countrate(get_tagger(), chans);
	if (state==STATE_STOPPED) {
		countrate->stop();
		countrate->clear();
	}

	out << JSKEY("result")
		<< "{}";
	return 200;
}

int BackendController::_Countrate::jsrpcGetData(JSON_FRAGMENT_STREAM &out, jsVar *) {
	std::vector<double> counts = countrate->Data();
	out << JSKEY("result")
		<< JSKEY( "countrate") << counts << ',';
	out << JSKEY("duration") << countrate->getDuration() << ',';
	out << "overflows" << countrate->getOverflows();
	return 200;
}


std::string BackendController::newCountrate(int chan) {
	return add_slot(new _Countrate(get_tagger(), chan));
}

BackendModule *BackendController::newCountrate(jsVar *parameter) {
	int chan;

	if (!parameter->getNumber("channel", chan)) chan=0;

	BackendModule *module=new _Countrate(get_tagger(), chan);
	module->set_common_config(parameter);
	return module;
}

/* --- */



BackendController::_Correlation::_Correlation(TimeTagger* tagger, int _channel, int _shot_trigger, int _length, long long _binwidth)
: BackendModule(tagger),
  length(_length), binwidth(_binwidth),
  channel(_channel), shot_trigger(_shot_trigger) {
	correlation=new Correlation(tagger, channel, shot_trigger, length, binwidth);

}

std::string BackendController::_Correlation::class_name() {
	return "correlation";
}

void BackendController::_Correlation::start() {
	if (correlation) correlation->start();
}

void BackendController::_Correlation::stop() {
	if (correlation) correlation->stop();
}

void BackendController::_Correlation::clear() {
	if (correlation) correlation->clear();
}

int BackendController::_Correlation::status() {
	if (correlation) return correlation->status();
	else return STATE_STOPPED;
}

int BackendController::_Correlation::jsonGetDescriptor(JSON_STREAM &out) {
	out << "[";
	out << JSON("name" << "channel" 	<< "type" << "channel" << "default" << 0 );
	out << ",";
	out << JSON("name" << "shot_trigger" 	<< "type" << "channel" << "default" << 7 );
	out << ",";
	out << JSON("name" << "length"	<< "type" << "int_number" << "default" << 1000 );
	out << ",";
	out << JSON("name" << "binwidth"	<< "type" << "long_number" << "default" << 1000 );
	out << "]";
	return HTTP_OK;
}

int BackendController::_Correlation::jsrpcGetDescriptor(JSON_FRAGMENT_STREAM &json, jsVar *parm) {
	json << JSKEY("result");
	jsonGetDescriptor(json.os);
	return HTTP_OK;
}

int BackendController::_Correlation::jsrpcGetConfig(JSON_FRAGMENT_STREAM &json, jsVar *) {
	json << JSKEY("result");
	jsonWriteConfig(json.os);
	return HTTP_OK;
}

void BackendController::_Correlation::jsonWriteConfig(JSON_STREAM &out) {
	out << JSON("name" << name
			<<	"id" << id
			<<	"classname" << "correlation"
			<<	"state" << status_str()
			<<	"channel" << channel
			<<	"shot_trigger" << shot_trigger
			<<	"binwidth" << binwidth
			<< 	"length" << length
		);
}

int BackendController::_Correlation::jsrpcSetConfig(JSON_FRAGMENT_STREAM &out, jsVar *parameter) {
	parameter->getNumber("channel", channel);
	parameter->getNumber("shot_trigger", shot_trigger);
	parameter->getNumber("length", length);
	parameter->getNumber("binwidth", binwidth);
	set_common_config(parameter);

	int state=correlation->status();
	delete correlation;

	correlation=new Correlation(get_tagger(), channel, shot_trigger, length, binwidth);
	if (state==STATE_STOPPED) {
		correlation->stop();
		correlation->clear();
	}

	out << "result" << JSEMPTY;
	return HTTP_OK;
}

#include <algorithm>
#include <climits>

int BackendController::_Correlation::jsrpcGetData(JSON_FRAGMENT_STREAM &json, jsVar *) {
	std::vector<int> data=correlation->Data();
	json << "result" << JSON(
					"data" << data
					);
	return HTTP_OK;
/*
	correlation->getData(&data, &count);
	out << JSKEY("result") << '{';
	int count;

	int tmin=INT_MAX;
	int tmax=INT_MIN;
	int points=0;
	int *p=data;
	for (int n=0; n<count; ++n) {
		if (p[n]>0) {
			++points;
			tmin = std::min(tmin,n);
			tmax = std::max(tmax,n);
		}
	}

	out << JSKEY("data")<< "[";
	bool first=true;
	p=data;
	for (int n=0; n<count; ++n) {
		if (n >= tmin && n<=tmax) {
			if (!first)
				out << ',';
			out << p[n];
			first=false;
		}
	}
	out << "],";

	out << JSKEY("duration") << correlation->getDuration() << ',';
	out << JSKEY("overflows") << correlation->getOverflows() << ',';
	//out << JSKEY("counts") <<  correlation->getCounts() << ',';

	out << JSKEY("points") <<  points << ',';
	out << JSKEY("tmin") <<  (binwidth*tmin) << ',';
	out << JSKEY("tstep") << binwidth << ',';
	out << JSKEY("tmax") <<  (binwidth*tmax) << '}';

	delete [] data;

	return 200;
*/
}

std::string BackendController::newCorrelation(int _channel, int _shot_trigger, int _length, long long _binwidth) {
	return add_slot(new _Correlation(get_tagger(),  _channel, _shot_trigger, _length, _binwidth));
}

BackendModule *BackendController::newCorrelation(jsVar *parameter) {
	int _length;
	long long _binwidth;
	int _sequence_length;
	int _channel;
	int _shot_trigger;
	int _sequence_trigger;

	if (!parameter->getNumber("channel", _channel)) _channel=0;
	if (!parameter->getNumber("sequence_length", _sequence_length)) _sequence_length=1000;
	if (!parameter->getNumber("length", _length)) _length=1000;
	if (!parameter->getNumber("binwidth", _binwidth)) _binwidth=1000;
	if (!parameter->getNumber("sequence_trigger", _sequence_trigger)) _sequence_trigger=-1;
	if (!parameter->getNumber("shot_trigger", _shot_trigger)) _shot_trigger=-1;

	BackendModule *module=new _Correlation(get_tagger(), _channel, _shot_trigger, _length, _binwidth);
	module->set_common_config(parameter);

	return module;
}




BackendController::_TimeDifferences::_TimeDifferences(TimeTagger* tagger, int _length, long long _binwidth, int _sequence_length, int _channel, int _shot_trigger, int _sequence_trigger)
: BackendModule(tagger),
  length(_length), binwidth(_binwidth), sequence_length(_sequence_length),
  channel(_channel), shot_trigger(_shot_trigger), sequence_trigger(_sequence_trigger) {
	right_correlation=new TimeDifferences(tagger, channel, shot_trigger, sequence_trigger, binwidth, length, sequence_length);
	left_correlation=new TimeDifferences(tagger, shot_trigger, channel, sequence_trigger, binwidth, length, sequence_length);

}

std::string BackendController::_TimeDifferences::class_name() {
	return "timedifferences";
}

void BackendController::_TimeDifferences::start() {
	if (right_correlation) right_correlation->start();
	if (left_correlation) left_correlation->start();
}

void BackendController::_TimeDifferences::stop() {
	if (right_correlation) right_correlation->stop();
	if (left_correlation) left_correlation->stop();
}

void BackendController::_TimeDifferences::clear() {
	if (right_correlation) right_correlation->clear();
	if (left_correlation) left_correlation->clear();
}

int BackendController::_TimeDifferences::status() {
	if (right_correlation) return right_correlation->status();
	else return STATE_STOPPED;
}

int BackendController::_TimeDifferences::jsonGetDescriptor(JSON_FRAGMENT_STREAM &out, jsVar*) {
	out << "[";
	out << JSON("name" << "channel" 	<< "type" << "channel" << "default" << 0 );
	out << ",";
	out << JSON("name" << "shot_trigger" 	<< "type" << "opt_channel" << "default" << 7 );
	out << ",";
	out << JSON("name" << "sequence_trigger" 	<< "type" << "opt_channel" << "default" << -1 );
	out << ",";
	out << JSON("name" << "length"	<< "type" << "int_number" << "default" << 1000 );
	out << ",";
	out << JSON("name" << "sequence_length"	<< "type" << "int_number" << "default" << 1 );
	out << ",";
	out << JSON("name" << "binwidth"	<< "type" << "long_number" << "default" << 1000 );
	out << "]";
	return 200;
}

int BackendController::_TimeDifferences::jsrpcGetDescriptor(JSON_FRAGMENT_STREAM &out, jsVar *parm) {
	out << JSKEY("result");
	jsonGetDescriptor(out, parm);
	return 200;
}

int BackendController::_TimeDifferences::jsrpcGetConfig(JSON_FRAGMENT_STREAM &out, jsVar *) {
	out << JSKEY("result");
	jsonWriteConfig(out);
	return 200;
}

void BackendController::_TimeDifferences::jsonWriteConfig(JSON_FRAGMENT_STREAM &out) {
	out << JSON("name" << name
			<<	"id" << id
			<<	"classname" << "timedifferences"
			<<	"state" << status_str()
			<<	"channel" << channel
			<<	"shot_trigger" << shot_trigger
			<<	"sequence_trigger" << sequence_trigger
			<<	"binwidth" << binwidth
			<< 	"length" << length
			<<	"sequence_length" << sequence_length
		);
}

int BackendController::_TimeDifferences::jsrpcSetConfig(JSON_FRAGMENT_STREAM &out, jsVar *parameter) {
	parameter->getNumber("channel", channel);
	parameter->getNumber("sequence_length", sequence_length);
	parameter->getNumber("length", length);
	parameter->getNumber("binwidth", binwidth);
	parameter->getNumber("sequence_trigger", sequence_trigger);
	parameter->getNumber("shot_trigger", shot_trigger);
	set_common_config(parameter);

	int state=right_correlation->status();
	delete right_correlation;
	delete left_correlation;

	right_correlation=new TimeDifferences(get_tagger(), channel, shot_trigger, sequence_trigger, binwidth, length, sequence_length);
	left_correlation=new TimeDifferences(get_tagger(), shot_trigger, channel, sequence_trigger, binwidth, length, sequence_length);
	if (state==STATE_STOPPED) {
		right_correlation->stop();
		right_correlation->clear();
		left_correlation->stop();
		left_correlation->clear();
	}

	out << JSKEY("result") << "{}";
	return 200;
}


int BackendController::_TimeDifferences::jsrpcGetData(JSON_FRAGMENT_STREAM &out, jsVar *param) {
	int count, dim;
	int *right_data, *left_data;
	right_correlation->getData(&right_data, &count, &dim);
	left_correlation->getData(&left_data, &count, &dim);

	int tmin=INT_MAX;
	int tmax=INT_MIN;
	int tstep=binwidth;

	bool normalize=true;
	int maxpoints=1000;
	int mvavg=0;
	param->getNumber("moving_average", mvavg);
	param->getBool("normalize", normalize);
	param->getNumber("maxpoints", maxpoints);

	double _tmin, _tmax;
	if (param->getNumber("tmin", _tmin)) {
		tmin=(_tmin / this->binwidth);
	}
	if (param->getNumber("tmax", _tmax)) {
		tmax=(_tmax / this->binwidth);
	}

	//std::cerr << "PARAM: " << param << std::endl;


	int *p1, *p2;
	if (tmin==INT_MAX) {
		p1=right_data;
		p2=left_data;
		for (int n=0; n<count; ++n) {
			for (int n=dim-1; n>=0; --n) {
				if (p2[n]>0) {
					tmin = std::min(tmin,-n);
				}
			}
			for (int n=0; n<dim; ++n) {
				if (p1[n]>0) {
					tmin = std::min(tmin,n);
				}
			}
			p1+=dim; p2+=dim;
		}
	}
	if (tmax==INT_MIN) {
		p1=right_data;
		p2=left_data;
		for (int n=0; n<count; ++n) {
			for (int n=dim-1; n>=0; --n) {
				if (p2[n]>0) {
					tmax = std::max(tmax,-n);
				}
			}
			for (int n=0; n<dim; ++n) {
				if (p1[n]>0) {
					tmax = std::max(tmax,n);
				}
			}
			p1+=dim; p2+=dim;
		}
	}

	int npoints=tmax-tmin+2;

	double *data=new double [count*(npoints*2+2)];
	p1=right_data;
	p2=left_data;
	double *p=data;
	int collected=0;
	for (int n=0; n<count; ++n) {
		double t=tmin*tstep;
		double tstep=binwidth;
		for (int i=dim-1; i>=0; --i) {
			if (-i >= tmin && -i<=tmax) {
				*p++ = t;
				//*p++ = (p2[i] * normalizer);
				*p++ = p2[i];
				t += tstep;
				collected+=2;
			}
		}
		for (int j=0; j<dim; ++j) {
			if (j >= tmin && j<=tmax) {
				*p++ = t;
				//*p++ = (p1[j] * normalizer);
				*p++ = p1[j];
				t+=tstep;
				collected+=2;
			}
		}
		p1+=dim; p2+=dim;
	}
//	std::cerr << " tmin:" << tmin << " tmax:" << tmax << " count:" << count;
//	std::cerr << " POINTS:" << npoints << " COLLECTED:" << collected << std::endl;

	if (maxpoints>0) {
		data=DiagramData::downsample2(data, count, npoints, maxpoints);
	}

	if (normalize) {
		double tmax=-1;
		for (int n=0; n<count; ++n) {
			for (int m=0; m<npoints; ++m) {
				tmax=std::max(tmax, data[n*npoints + m*2 + 1]);
			}
		}
		for (int n=0; n<count; ++n) {
			for (int m=0; m<npoints; ++m) {
				data[n*npoints + m*2 + 1] /= tmax;
			}
		}

	}

	//if (mvavg > 0)
	//	movingAverage2(data, count, npoints, mvavg);

	out << JSKEY("result") << '{';
	out << JSKEY("data")<< "[";
	p=data;
	for (int n=0; n<count; ++n) {
		if (n>0)
			out << ',';

		out << '[';

		for (int m=0; m<npoints; ++m) {
			if (m>0) out << ',';
			out << '[' << (p[0]) << ',' << (p[1]) << ']';
			p+=2;
		}

		out << ']';
		p1+=dim; p2+=dim;
	}
	out << "],";

	out << JSKEY("duration") << right_correlation->getDuration() << ',';
	out << JSKEY("overflows") << right_correlation->getOverflows() << ',';
	out << JSKEY("counts") <<  (right_correlation->getCounts()+left_correlation->getCounts())/2 << ',';

	out << JSKEY("points") <<  npoints << ',';
	out << JSKEY("tmin") <<  (tmin*binwidth) << ',';
	out << JSKEY("tstep") << (tstep*binwidth) << ',';
	out << JSKEY("tmax") <<  tmax << '}';

	delete [] data;
	delete [] right_data;
	delete [] left_data;
	return 200;
}

std::string BackendController::newTimeDifferences(int _length, long long _binwidth, int _sequence_length, int _channel, int _shot_trigger, int _sequence_trigger) {
	return add_slot(new _TimeDifferences(get_tagger(),  _length, _binwidth, _sequence_length, _channel, _shot_trigger, _sequence_trigger));
}

BackendModule *BackendController::newTimeDifferences(jsVar *parameter) {
	int _length;
	long long _binwidth;
	int _sequence_length;
	int _channel;
	int _shot_trigger;
	int _sequence_trigger;

	if (!parameter->getNumber("channel", _channel)) _channel=0;
	if (!parameter->getNumber("sequence_length", _sequence_length)) _sequence_length=1000;
	if (!parameter->getNumber("length", _length)) _length=1000;
	if (!parameter->getNumber("binwidth", _binwidth)) _binwidth=1000;
	if (!parameter->getNumber("sequence_trigger", _sequence_trigger)) _sequence_trigger=-1;
	if (!parameter->getNumber("shot_trigger", _shot_trigger)) _shot_trigger=-1;

	BackendModule *module=new _TimeDifferences(get_tagger(),  _length, _binwidth, _sequence_length, _channel, _shot_trigger, _sequence_trigger);
	module->set_common_config(parameter);

	return module;
}




BackendController::_Flim::		_Flim(TimeTagger* tagger,
		int _click_channel, int _start_channel, int _pixel_channel,
        long long _binwidth, int _bins,
        int _pixels, long long _pixel_time)
: BackendModule(tagger),
  click_channel(_click_channel), start_channel(_start_channel), pixel_channel(_pixel_channel),
  binwidth(_binwidth), bins(_bins),
  pixels(_pixels), pixel_time(pixel_time) {

	flim=new Flim(tagger, click_channel, start_channel, pixel_channel, binwidth, bins, pixels, pixel_time);
}

std::string BackendController::_Flim::class_name() {
	return "flim";
}

void BackendController::_Flim::start() {
	if (flim) flim->start();
}

void BackendController::_Flim::stop() {
	if (flim) flim->stop();
}

void BackendController::_Flim::clear() {
	if (flim) flim->clear();
}

int BackendController::_Flim::status() {
	if (flim) return flim->status();
	else return STATE_STOPPED;
}

int BackendController::_Flim::jsonGetDescriptor(JSON_STREAM &out) {
	out << "[";
	out << JSON("name" << "click_channel" 	<< "type" << "channel" << "default" << 0 );
	out << ",";
	out << JSON("name" << "start_channel" 	<< "type" << "channel" << "default" << 7 );
	out << ",";
	out << JSON("name" << "pixel_channel" 	<< "type" << "opt_channel" << "default" << -1 );
	out << ",";
	out << JSON("name" << "binwidth"	<< "type" << "long_number" << "default" << 1000 );
	out << ",";
	out << JSON("name" << "bins"	<< "type" << "int_number" << "default" << 1000 );
	out << ",";
	out << JSON("name" << "pixels"	<< "type" << "int_number" << "default" << 1000 );
	out << ",";
	out << JSON("name" << "pixel_time"	<< "type" << "long_number" << "default" << 1000000 );
	out << "]";
	return 200;
}

int BackendController::_Flim::jsrpcGetDescriptor(JSON_FRAGMENT_STREAM &out, jsVar *parm) {
	out << JSKEY("result");
	jsonGetDescriptor(out, parm);
	return 200;
}

int BackendController::_Flim::jsrpcGetConfig(JSON_FRAGMENT_STREAM &out, jsVar *) {
	out << JSKEY("result");
	jsonWriteConfig(out);
	return 200;
}

void BackendController::_Flim::jsonWriteConfig(JSON_STREAM &out) {
	out << JSON("name" << name
			<<	"id" << id
			<<	"classname" << "flim"
			<<	"state" << status_str()

			<<	"click_channel" << click_channel
			<<	"start_channel" << start_channel
			<<	"pixel_channel" << pixel_channel

			<<	"binwidth" << binwidth
			<<	"bins" << bins
			<<	"pixels" << pixels
			<< 	"pixel_time" << pixel_time
		);
}

int BackendController::_Flim::jsrpcSetConfig(JSON_FRAGMENT_STREAM &out, jsVar *parameter) {
	parameter->getNumber("click_channel", click_channel);
	parameter->getNumber("start_channel", start_channel);
	parameter->getNumber("pixel_channel", pixel_channel);
	parameter->getNumber("binwidth", binwidth);
	parameter->getNumber("bins", bins);
	parameter->getNumber("pixels", pixels);
	parameter->getNumber("pixel_time", pixel_time);
	set_common_config(parameter);

	int state=flim->status();
	delete flim;

	flim=new Flim(get_tagger(), click_channel, start_channel, pixel_channel, binwidth, bins, pixels, pixel_time);
	if (state==STATE_STOPPED) {
		flim->stop();
		flim->clear();
	}

	out << JSKEY("result") << "{}";
	return 200;
}

#include <algorithm>
#include <climits>

int BackendController::_Flim::jsrpcGetData(JSON_FRAGMENT_STREAM &out, jsVar *) {
	int count, dim;
	int *data;
	flim->getData(&data, &count, &dim);

std::cerr << "COUNT:" << count << " DIM:"<< dim << std::endl;

	out << JSKEY("result") << '{';

	out << JSKEY("data")<< "[";

	int tmin=INT_MAX;
	int tmax=INT_MIN;
	int *p=data;
	for (int n=0; n<count; ++n) {
		for (int n=0; n<dim; ++n) {
			if (p[n]>0) {
				tmin = std::min(tmin,n);
				tmax = std::max(tmax,n);
			}
		}
		p+=dim;
	}
std::cerr << "TMIN:" << tmin << " TMAX:" << tmax << std::endl;

	p=data;
	for (int n=0; n<count; ++n) {
		if (n>0)
			out << ',';
		out << '[';

		bool first=true;
		for (int j=0; j<dim; ++j) {
			//if (j >= tmin && j<=tmax) {
				if (!first) out << ',';
				first=false;
				out << p[j];
			//}
		}

		out << ']';
		p+=dim;
	}
	out << "],";

	out << JSKEY("duration") << flim->getDuration() << ',';
	out << JSKEY("overflows") << flim->getOverflows() << ',';
	//out << JSKEY("counts") <<  flim->getCounts() << ',';

	out << JSKEY("points") <<  ((tmin - tmax) + 2) << ',';
	out << JSKEY("tmin") <<  (binwidth*tmin) << ',';
	out << JSKEY("tstep") << binwidth << ',';
	out << JSKEY("tmax") <<  (binwidth*tmax) << '}';

	delete [] data;
	return 200;
}

std::string BackendController::newFlim(int click_channel, int start_channel, int pixel_channel,
		                            long long binwidth, int bins, int pixels, long long pixel_time) {
	return add_slot(new _Flim(get_tagger(), click_channel, start_channel, pixel_channel, binwidth, bins, pixels, pixel_time));
}

BackendModule *BackendController::newFlim(jsVar *parameter) {
	int _click_channel;
	int _start_channel;
	int _pixel_channel;
	long long _binwidth;
	int _bins;
	int _pixels;
	long long _pixel_time;

	if (!parameter->getNumber("click_channel", _click_channel)) _click_channel=0;
	if (!parameter->getNumber("start_channel", _start_channel)) _start_channel=0;
	if (!parameter->getNumber("pixel_channel", _pixel_channel)) _pixel_channel=0;

	if (!parameter->getNumber("binwidth", _binwidth)) _binwidth=1000;
	if (!parameter->getNumber("bins", _bins)) _bins=1000;
	if (!parameter->getNumber("pixels", _pixels)) _pixels=1000;
	if (!parameter->getNumber("pixel_time", _pixel_time)) _pixel_time=100;

	BackendModule *module=new _Flim(get_tagger(), _click_channel, _start_channel, _pixel_channel, _binwidth, _bins, _pixels, _pixel_time);
	module->set_common_config(parameter);

	return module;
}

#endif
