/*
 * BackendControl.h
 *
 *  Created on: 23.04.2013
 *      Author: mag
 */

#ifndef BACKENDCONTROL_H_
#define BACKENDCONTROL_H_

#include "httpd/json.h"
#include "ModuleController.h"
#include "../backend/TimeTagger.h"
#include "../backend/Counter.cpp"





class BackendController {

public:
	BackendController(const std::string &serial);
	BackendController(const std::string &serial, jsVar *config);
	~BackendController();

	void init(std::string);
	void registerModule(ModuleController::AbstractFactory *);

	std::string get_serial();
	std::string get_name();
	std::string get_hardware_name();
	std::string get_status_tag();
	long long get_overflows();

protected:
	TimeTagger *get_tagger();

	std::string add_slot(ModuleController *);
	int get_module_count();
	ModuleController *get_module(std::string id);
	void remove_module(const std::string &id);



protected:
	std::string _name;
	TimeTagger *_tagger;
	std::map<std::string, ModuleController *>slots;

	std::map<std::string, ModuleController::AbstractFactory *> module_factories;



public:


	ModuleController *get_module(JSON_FRAGMENT_STREAM &out, jsVar *);

	int jsonGetInfo(JSON_STREAM &);
	int jsonWriteClasses(JSON_STREAM &);
	int jsrpcGetModules(JSON_STREAM &);

	void jsonWriteConfig(JSON_STREAM &out);


	int jsrpcGetInfo(JSON_FRAGMENT_STREAM &, jsVar *parm);
	int jsrpcRename(JSON_FRAGMENT_STREAM &, jsVar *parm);

	int jsrpcGetDACSettings(JSON_FRAGMENT_STREAM &, jsVar *parm);
	int jsrpcSetDACValue(JSON_FRAGMENT_STREAM &, jsVar *parm);
	int jsrpcGetFilter(JSON_FRAGMENT_STREAM &, jsVar *parm);
	int jsrpcSetFilter(JSON_FRAGMENT_STREAM &, jsVar *parm);
	int jsrpcGetTestsignal(JSON_FRAGMENT_STREAM &, jsVar *parm);
	int jsrpcSetTestsignal(JSON_FRAGMENT_STREAM &, jsVar *parm);
	int jsrpcGetLineDelay(JSON_FRAGMENT_STREAM &, jsVar *parm);
	int jsrpcSetLineDelay(JSON_FRAGMENT_STREAM &, jsVar *parm);

	int jsrpcGetClasses(JSON_FRAGMENT_STREAM &, jsVar *parm);

	int jsrpcGetModules(JSON_FRAGMENT_STREAM &, jsVar *parm);

	int jsrpcCreateModule(JSON_FRAGMENT_STREAM &, jsVar *parm);

	int jsrpcStartModule(JSON_FRAGMENT_STREAM &, jsVar *parm);
	int jsrpcStopModule(JSON_FRAGMENT_STREAM &, jsVar *parm);
	int jsrpcClearModule(JSON_FRAGMENT_STREAM &, jsVar *parm);

	int jsrpcRemoveModule(JSON_FRAGMENT_STREAM &, jsVar *parm);

	int jsrpcGetModuleDescriptor(JSON_FRAGMENT_STREAM &, jsVar *parm);
	int jsrpcGetModuleConfig(JSON_FRAGMENT_STREAM &, jsVar *parm);
	int jsrpcSetModuleConfig(JSON_FRAGMENT_STREAM &, jsVar *parm);

	int jsrpcGetData(JSON_FRAGMENT_STREAM &, jsVar *parm);


	ModuleController *newModule(jsVar *);


	class _Countrate : public ModuleController {
	public:
		_Countrate(TimeTagger* tagger, int _chan);

		static int jsonGetDescriptor(JSON_FRAGMENT_STREAM &, jsVar *parm);

		virtual std::string class_name();
		virtual void start();
		virtual void stop();
		virtual void clear();
		virtual int status();

		virtual int jsrpcGetDescriptor(JSON_FRAGMENT_STREAM &, jsVar *parm);
		virtual int jsrpcGetConfig(JSON_FRAGMENT_STREAM &, jsVar *parm);
		virtual int jsrpcSetConfig(JSON_FRAGMENT_STREAM &, jsVar *parm);
		virtual void jsrpcGetMetaData(JSON_FRAGMENT_STREAM &, std::vector<int> &);
		virtual int jsrpcGetData(JSON_FRAGMENT_STREAM &, jsVar *parm);

		void jsonWriteConfig(JSON_STREAM &out);

	protected:
		Countrate *countrate;

		int channel;
	};
	ModuleController *newCountrate(jsVar *parameter);






/*
	class _TimeDifferences : public BackendModule {
	public:
		_TimeDifferences(TimeTagger* tagger, int _length, long long _binwidth, int _sequence_length, int _channel, int _shot_trigger, int _sequence_trigger);

		static int jsonGetDescriptor(JSON_STREAM &out);

		virtual std::string class_name();
		virtual void start();
		virtual void stop();
		virtual void clear();
		virtual int status();

		virtual int jsrpcGetDescriptor(JSON_FRAGMENT_STREAM &out, jsVar *params);
		virtual int jsrpcGetConfig(JSON_FRAGMENT_STREAM &out, jsVar *params);
		virtual int jsrpcSetConfig(JSON_FRAGMENT_STREAM &out, jsVar *params);
		virtual int jsrpcGetData(JSON_FRAGMENT_STREAM &out, jsVar *params);

		void jsonWriteConfig(JSON_FRAGMENT_STREAM &out);

	protected:
		TimeDifferences *right_correlation;
		TimeDifferences *left_correlation;
		int length;
		long long binwidth;
		int sequence_length;
		int channel;
		int shot_trigger;
		int sequence_trigger;
	};
	std::string newTimeDifferences(int _length, long long _binwidth, int _sequence_length, int _channel, int _shot_trigger, int _sequence_trigger);
	BackendModule *newTimeDifferences(jsVar *parameter);

	class _Correlation : public BackendModule {
	public:
		_Correlation(TimeTagger* tagger, int _channel, int _shot_trigger, int _length, long long _binwidth);

		static int jsonGetDescriptor(JSON_STREAM &out);

		virtual std::string class_name();
		virtual void start();
		virtual void stop();
		virtual void clear();
		virtual int status();

		virtual int jsrpcGetDescriptor(JSON_FRAGMENT_STREAM &out, jsVar *params);
		virtual int jsrpcGetConfig(JSON_FRAGMENT_STREAM &out, jsVar *params);
		virtual int jsrpcSetConfig(JSON_FRAGMENT_STREAM &out, jsVar *params);
		virtual int jsrpcGetData(JSON_FRAGMENT_STREAM &out, jsVar *params);

		void jsonWriteConfig(JSON_STREAM &out);

	protected:
		Correlation *correlation;
		int length;
		long long binwidth;
		int channel;
		int shot_trigger;
	};
	std::string newCorrelation(int _channel, int _shot_trigger, int _length, long long _binwidth);
	BackendModule *newCorrelation(jsVar *parameter);


	class _Flim : public BackendModule {
	public:
		_Flim(TimeTagger* tagger,
				int _click_channel, int _start_channel, int _pixel_channel=-1,
	            long long _binwidth=100, int _bins=1000,
	            int _pixels=1, long long _pixel_time=-1);

		static int jsonGetDescriptor(JSON_STREAM &out);

		virtual std::string class_name();
		virtual void start();
		virtual void stop();
		virtual void clear();
		virtual int status();

		virtual int jsrpcGetDescriptor(JSON_FRAGMENT_STREAM &out, jsVar *params);
		virtual int jsrpcGetConfig(JSON_FRAGMENT_STREAM &out, jsVar *params);
		virtual int jsrpcSetConfig(JSON_FRAGMENT_STREAM &out, jsVar *params);
		virtual int jsrpcGetData(JSON_FRAGMENT_STREAM &out, jsVar *params);

		void jsonWriteConfig(JSON_STREAM &out);

	protected:
		Flim *flim;
		int click_channel;
		int start_channel;
		int pixel_channel;
		long long binwidth;
		int bins;
		int pixels;
		long long pixel_time;
	};

	std::string newFlim(int _click_channel, int _start_channel, int _pixel_channel=-1,
			            long long _binwidth=100, int _bins=1000,
			            int _pixels=1, long long _pixel_time=-1);
	BackendModule *newFlim(jsVar *parameter);
*/

};


#endif /* BACKENDCONTROL_H_ */
