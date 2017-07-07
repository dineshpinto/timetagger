/*
    backend for TimeTagger, an OpalKelly based single photon counting library
    Copyright (C) 2011  Markus Wick <wickmarkus@web.de>

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License along
    with this program; if not, write to the Free Software Foundation, Inc.,
    51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
*/
#ifndef TIMETAGGER_H
#define TIMETAGGER_H

#include "TimetaggerFPGA.h"
#include "PRBS.h"

#include <thread>
#include <mutex>
#include <string>
#include <vector>
#include <queue>
#include <climits>
#include "okFrontPanelDLL.h"

#include "TimeTagger.h"

#define override	// disable c++11 for now


const std::string bitfilename = "TimeTaggerController.bit";		// location of bitfile for fpga

const int workers = 4; 											// count of threads
																// *2 ist for up/down
const int distribution = 1<<7;									// 7 Bit für Zeitauflösung

const int blocksize = 1024;										// size of data, transmitted at once [byte]
const int buffersize = blocksize * 128;						// data to read and convert at once [byte]


//const int DAC_channel_map [8] = {3,2,1,0,4,5,6,7}; // AD5318
const int DAC_channel_map [8] = {7,5,3,1,6,4,2,0}; // DAC8568


static const channel_t channels = 8*2;        // number of hardware channels. musst be the same as on the FPGA
static const timestamp_t picoseconds = 6000;   // time delay for each macro cycle

static const timestamp_t default_channel_delay[16] = {565, 976, 458, 0, 31, 698, 50, 273, 565, 976, 458, 0, 31, 698, 50, 273};
//static const timestamp_t default_channel_delay[16] = {0};

class _Iterator;
class TimeTagger;
class _Worker;
class _Iter;

union NativeTag {
	struct {
		uint32_t dist     :  8;
		uint32_t chan     :  4;
		uint32_t unused   :  1;
		uint32_t edge     :  1;
		uint32_t overflow :  1;
		uint32_t rollover :  1;
		uint32_t time     : 16;
	};
	uint32_t hex;
};
static_assert(sizeof(NativeTag) == 4, "NativeTag must be 4 bytes long");

class _TimeTagger : public TimeTagger {
public:
	_TimeTagger(std::string serial="");
	~_TimeTagger();

	void setTriggerLevel(channel_t channel, double voltage) override;
	double getTriggerLevel(channel_t channel) override;

	std::vector<double> getTriggerLevel();

	void setFilter(bool state) override;
	bool getFilter() override;

	//--------------------
	//added 04.09.2015, n.abt --> for bitstream and jump address for AWG
	//--------------------

	/**
	 *
	 */
	void setAWGDataOne(int data_one) override;
	void setAWGDataTwo(int data_two) override;

	//--------------------

	void setNormalization(bool state) override;
	bool getNormalization() override;

	timestamp_t setDeadtime(channel_t chan, timestamp_t deadtime) override;

	void setLineDelay(channel_t channel, timestamp_t time) override;
	timestamp_t getLineDelay(channel_t channel) override;
	
	std::vector<timestamp_t> getLineDelay();

	_Iter* addIterator(_Iterator *it) override;

	void registerChannel(channel_t chan) override;
	void unregisterChannel(channel_t chan) override;

	void setTestsignal(channel_t chan, bool enable) override;
	bool getTestsignal(channel_t chan) override;

	void autoCalibration(bool verbose = true) override;

	void getDistributionCount(channel_t chan, long long **ARGOUTVIEWM_ARRAY1, int *DIM1) override;
	void getDistributionPSecs(channel_t chan, timestamp_t **ARGOUTVIEWM_ARRAY1, int *DIM1) override;

	virtual channel_t getChannels() override;
	virtual timestamp_t getPsPerClock() override;

	void sync() override;


	int getBoardModel() override;
	enum running_state getStatus() override;
	std::string getSerial() override;

	// Used by workers
	void iter(_Worker *w);

protected:
	void _next(_Worker *w, _Iterator *i);

private:

	std::string serial;

	double trigger_level [8];
	bool filter;
	uint16_t deadtimes [16];

	_Worker* worker[workers];

	void configureFpga();
	void configureChannel();
	void updateDeadtimes();
	void updateCalibrations();

	void read(_Worker *w);
	void convert(_Worker *w);

	void queue_init();

	timestamp_t tag_delay[16];

	class TagCompare {
	public:
		bool operator()(const Tag& a, const Tag& b) const {
			return a.time - b.time > 0; // Note: this isn't the same as a>b because of the s64 overflow
		}
	};
	std::priority_queue<Tag, std::vector<Tag>, TagCompare> tag_queue;

	void warning(int lvl, const char* msg);
	void update_distribution();

	// FPGA Connection
	TimetaggerFPGA *fpga;

	int chans[channels];
	bool fpga_channel_reconfig;
	int calibration_enabled;

	std::mutex channel_mutex;

	// Buffer Mutexes
	std::mutex fpga_mutex;
	std::mutex read_mutex;
	std::mutex convert_mutex;

	// Convert Data
	timestamp_t rollover;
	long long distribution_counts[channels][distribution];
	timestamp_t distribution_ps[channels][distribution+1];
	PRBS prbs;
	bool distribute_tags;

	// iterator
	_Iter* iterators;
};

class _Worker {
public:
	_Worker(_TimeTagger* t);
	~_Worker();

	void terminate() {
		run = false;
		thread.join();
	}

	// Buffer between FPGA-Fifo and PC
	std::vector<NativeTag> ibuffer;

	// Buffer after convertion
	std::vector<Tag> obuffer;
	timestamp_t time;

private:
	void work();
	static void start(_Worker *w);

	// worker thread
	bool run;
	_TimeTagger* tagger;
	std::thread thread;

};

class _Iter {
public:
	_Iter() { next=0; iter=0; };

	_Iter* next;
	_Iterator* iter;

	std::mutex* getMutex() { return &mutex; };

private:
	std::mutex mutex;
};


#endif
