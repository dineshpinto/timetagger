
#ifndef MODULECONTROLLER_H
#define MODULECONTROLLER_H


#include "httpd/json.h"
#include "../backend/TimeTagger.h"

class BackendController;

class ModuleController {
	friend class BackendController;

public:
	class AbstractFactory {
	public:
		AbstractFactory() {}
		virtual ~AbstractFactory() {}
		virtual std::string classname() = 0;
		virtual ModuleController *create(TimeTagger *, jsVar *parameter) = 0;
		virtual int jsonWriteDescriptor(JSON_STREAM &) = 0;
	};

public:
	ModuleController(TimeTagger* _tagger) : tagger(_tagger) { }
	virtual ~ ModuleController() {}

	virtual std::string classname() = 0;

	void start();
	void stop();
	void clear();
	int status();
	long getOverflows();
	timestamp_t getDuration();

	std::string status_str();

	int jsonGetStatus(JSON_STREAM &);

	virtual int jsrpcGetDescriptor(JSON_FRAGMENT_STREAM &, jsVar *parm) = 0;
	virtual int jsrpcGetConfig(JSON_FRAGMENT_STREAM &, jsVar *parm) = 0;
	virtual int jsrpcSetConfig(JSON_FRAGMENT_STREAM &, jsVar *parm) = 0;
	virtual int jsrpcGetData(JSON_FRAGMENT_STREAM &, jsVar *parm) = 0;

	virtual void jsonWriteConfig(JSON_STREAM &out) = 0;

	void set_common_config(jsVar *);

protected:
	std::string get_id() { return id; }
	TimeTagger* get_tagger() { return tagger; }
	virtual _Iterator* get_iterator() = 0;

	std::string name;

private:
	std::string id;
	TimeTagger* tagger;
};

#endif
