#include "../ModuleController.h"
#include "../../backend/Counter.cpp"

class CounterModule : public ModuleController {

public:
	class Factory : public AbstractFactory {
	public:
		virtual std::string classname();
		virtual ModuleController *create(TimeTagger *, jsVar *parameter);
		virtual int jsonWriteDescriptor(JSON_STREAM &);
	};
	friend class Factory;

public:
	CounterModule(TimeTagger* tagger, jsVar *);
	CounterModule(TimeTagger* tagger, channel_t _chan, timestamp_t _binwidth, int _bins);
	virtual ~CounterModule();

	virtual std::string classname();

	virtual int jsrpcGetDescriptor(JSON_FRAGMENT_STREAM &, jsVar *parm);
	virtual int jsrpcGetConfig(JSON_FRAGMENT_STREAM &, jsVar *parm);
	virtual int jsrpcSetConfig(JSON_FRAGMENT_STREAM &, jsVar *parm);
	virtual void jsrpcGetMetaData(JSON_FRAGMENT_STREAM &, std::vector<int> &);
	virtual int jsrpcGetData(JSON_FRAGMENT_STREAM &, jsVar *parm);

	void jsonWriteConfig(JSON_STREAM &out);
	void jsonWriteDescriptor(JSON_STREAM &out);


protected:
	virtual Counter *get_iterator() { return counter; }

	Counter *counter;

	channel_t channel;
	timestamp_t binwidth;
	int bins;

public:
	static Factory _factory;
};
