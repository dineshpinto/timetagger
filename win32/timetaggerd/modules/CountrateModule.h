
#include "../ModuleController.h"
#include "../../backend/Counter.cpp"

class CountrateModule : public ModuleController {

public:
	class Factory : public AbstractFactory {
	public:
		virtual std::string classname();
		virtual ModuleController *create(TimeTagger *, jsVar *parameter);
		virtual int jsonWriteDescriptor(JSON_STREAM &);
	};
	friend class Factory;

public:
	CountrateModule(TimeTagger* tagger, jsVar *);
	CountrateModule(TimeTagger* tagger, channel_t _chan);
	virtual ~CountrateModule();

	virtual std::string classname();

	virtual int jsrpcGetDescriptor(JSON_FRAGMENT_STREAM &, jsVar *parm);
	virtual int jsrpcGetConfig(JSON_FRAGMENT_STREAM &, jsVar *parm);
	virtual int jsrpcSetConfig(JSON_FRAGMENT_STREAM &, jsVar *parm);
	virtual void jsrpcGetMetaData(JSON_FRAGMENT_STREAM &, std::vector< double > &);
	virtual int jsrpcGetData(JSON_FRAGMENT_STREAM &, jsVar *parm);

	void jsonWriteConfig(JSON_STREAM &out);
	void jsonWriteDescriptor(JSON_STREAM &out);


protected:
	virtual Countrate *get_iterator() { return countrate; }

	Countrate *countrate;

	channel_t channel;

public:
	static Factory _factory;
};
