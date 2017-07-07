#include "../ModuleController.h"
#include "../../backend/CountBetweenMarkers.cpp"

class CountbetweenmarkersModule : public ModuleController {

public:
	class Factory : public AbstractFactory {
	public:
		virtual std::string classname();
		virtual ModuleController *create(TimeTagger *, jsVar *parameter);
		virtual int jsonWriteDescriptor(JSON_STREAM &);
	};
	friend class Factory;

public:
	CountbetweenmarkersModule(TimeTagger* tagger, jsVar *);
	CountbetweenmarkersModule(TimeTagger* tagger, int _bins, channel_t _click_channel, channel_t _marker_channel, channel_t _stop_channel);
	virtual ~CountbetweenmarkersModule();

	virtual std::string classname();

	virtual int jsrpcGetDescriptor(JSON_FRAGMENT_STREAM &, jsVar *parm);
	virtual int jsrpcGetConfig(JSON_FRAGMENT_STREAM &, jsVar *parm);
	virtual int jsrpcSetConfig(JSON_FRAGMENT_STREAM &, jsVar *parm);
	virtual void jsrpcGetMetaData(JSON_FRAGMENT_STREAM &, std::vector< int > &);
	virtual int jsrpcGetData(JSON_FRAGMENT_STREAM &, jsVar *parm);

	void jsonWriteConfig(JSON_STREAM &out);
	void jsonWriteDescriptor(JSON_STREAM &out);


protected:
	virtual CountBetweenMarkers *get_iterator() { return counter; }

	CountBetweenMarkers *counter;

	int bins;
	channel_t click_channel;
	channel_t marker_channel;
	channel_t stop_channel;

public:
	static Factory _factory;
};
