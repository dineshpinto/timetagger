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
#include <iostream>
#include <stdexcept>
#include <assert.h>
#ifndef _MSC_VER
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#endif
#include <stdlib.h>
#include "okFrontPanelDLL.h"
#include "Logger.h"
#include <math.h>

#include "backend.h"

static void error(const char *fmt, ...) {
	va_list va;
	va_start(va, fmt);
	Logger::vlog(Logger::BACKEND, Logger::LOG_ERROR, fmt, va);
}


static void info(const char *fmt, ...) {
	va_list va;
	va_start(va, fmt);
	Logger::vlog(Logger::BACKEND, Logger::LOG_INFO, fmt, va);
}

static void warning(const char *fmt, ...) {
	va_list va;
	va_start(va, fmt);
	Logger::vlog(Logger::BACKEND, Logger::LOG_WARNING, fmt, va);
}

_TimeTagger::_TimeTagger(std::string serial)
	{

	// remember serial number of FPGA
	this->serial = serial;

	// begin with filter disabled
	filter = 0;
	distribute_tags = false;

	// begin with trigger levels set to 0.5 V
	//for (auto & elem : trigger_level) elem = 0.5;
	for (size_t n=0; n < (sizeof(trigger_level)/sizeof(*trigger_level)); ++n)
		trigger_level[n]= 0.5;


	//for (auto& elem : deadtimes) elem = 1;
	for (size_t n=0; n<sizeof(deadtimes)/sizeof(*deadtimes); ++n) {
		deadtimes[n]=1;
	}

	// begin with an empty channel list and empty dist
	//for (auto& elem : chans) elem = 0;
	//for (auto& elems : distribution_counts)
	//	for (auto& elem : elems)
	//		elem = 0;
	for (size_t n=0; n<sizeof(distribution_counts)/sizeof(**distribution_counts); ++n)
		((long long *)distribution_counts)[n]=0;
	rollover = 0;

	// set all pointers to zero
	iterators = 0; //nullptr;
	fpga = new TimetaggerFPGA(serial);
	fpga_mutex.lock();
	configureFpga();
	fpga_mutex.unlock();

	calibration_enabled=0;

	fpga_channel_reconfig = 1;

	this->queue_init();

	// create worker
	for (size_t n=0; n<sizeof(worker)/sizeof(*worker); ++n) {
		worker[n]=new _Worker(this);
	}

	autoCalibration(false);
}

_TimeTagger::~_TimeTagger() {
	//terminate the worker
	for (size_t n=0; n<sizeof(worker)/sizeof(*worker); ++n) {
		_Worker *elem=worker[n];
		if (elem) {
			elem->terminate();
			delete elem;
			worker[n] = 0; // nullptr;
		}
	}

	// delete all iterators
	_Iter* iter = iterators;
	while(iter) {
		iter->iter->tagger = 0; //nullptr;
		iter = iter->next;
	}

	delete fpga;
}


_Iter* _TimeTagger::addIterator(_Iterator *it) {
	if(!it)	warning(0, "Tried to add an empty Iterator");

	_Iter * i = new _Iter();
	i->iter = it;

	convert_mutex.lock();
	i->next = iterators;
	iterators = i;
	convert_mutex.unlock();

	return i;
}

void _TimeTagger::registerChannel(channel_t chan) {
	if (chan >= channels) warning( 0, "Tried to register to unknown channel ");
	channel_mutex.lock();

	if((++chans[chan]) == 1)
		fpga_channel_reconfig = 1;

	channel_mutex.unlock();
}

void _TimeTagger::unregisterChannel(channel_t chan) {
	if (chan >= channels) warning(0, "Tried to unregister from unknown channel" );
	channel_mutex.lock();

	if((--chans[chan]) == 0)
		fpga_channel_reconfig = 1;

	channel_mutex.unlock();
}

void _TimeTagger::configureFpga () {
	fpga->configure();
    updateDeadtimes();
    updateCalibrations();
	fpga_channel_reconfig = 1;
}

int _TimeTagger::getBoardModel() {
	return fpga->getModel();
}

enum running_state _TimeTagger::getStatus() {
	if (!fpga->configured()) {
		return STATE_ERROR;
	}
	if (!this->worker[0]) {
		return STATE_STOPPED;
	} else {
		if (iterators)
			return STATE_RUNNING;
		else
			return STATE_IDLE;
	}

}

std::string _TimeTagger::getSerial() {
	fpga_mutex.lock();
	std::string serial = fpga->getSerial();
	fpga_mutex.unlock();
	return serial;
}

void _TimeTagger::setTriggerLevel(channel_t channel, double voltage) {
	fpga_mutex.lock();
	fpga->setTriggerLevel(channel, voltage);
	fpga_mutex.unlock();
	trigger_level[channel] = voltage;
}

double _TimeTagger::getTriggerLevel(channel_t channel) {
	return trigger_level[channel];
}

std::vector<double> _TimeTagger::getTriggerLevel() {
	std::vector<double> ret;
	for (size_t n=0; n<sizeof(trigger_level)/sizeof(*trigger_level); ++n)
		ret.push_back(trigger_level[n]);
	return ret;
}

void _TimeTagger::setFilter(bool state) {
	fpga_mutex.lock();
	fpga->setFilter(state);
	fpga_mutex.unlock();
	filter = state;
}

bool _TimeTagger::getFilter() {
	return filter;
}

//--------------------
//added 04.09.2015, n.abt --> for bitstream and jump address for AWG
//--------------------

void _TimeTagger::setAWGDataOne(int data_one) {
	fpga_mutex.lock();
	fpga->setAWGDataOne(data_one);
	fpga_mutex.unlock();
}

void _TimeTagger::setAWGDataTwo(int data_two) {
	fpga_mutex.lock();
	fpga->setAWGDataOne(data_two);
	fpga_mutex.unlock();
}

//--------------------


void _TimeTagger::setNormalization(bool state) {
	convert_mutex.lock();
	distribute_tags = state;
	convert_mutex.unlock();
}

bool _TimeTagger::getNormalization() {
	return distribute_tags;
}

timestamp_t _TimeTagger::setDeadtime(channel_t chan, timestamp_t deadtime)
{
    if (chan >= channels) {
        return 0;
    }

    // round to nearest integer
    timestamp_t clocks = (deadtime + picoseconds / 2) / picoseconds;

    // don't overflow the u16 value
    clocks = std::min<timestamp_t>(clocks, 0xFFFF);

    // we need at lest one clk
    clocks = std::max<timestamp_t>(clocks, 1);
    deadtimes[chan] = clocks;

    fpga_mutex.lock();
    updateDeadtimes();
    fpga_mutex.unlock();

    return clocks * picoseconds;
}

void _TimeTagger::updateDeadtimes()
{
    for (channel_t i=0; i<channels; i++) {
        fpga->setWireIn(ADDR_DEADTIME + i, deadtimes[i]);
    }
    fpga->UpdateWireIns();
}

void _TimeTagger::configureChannel() {
	channel_mutex.lock();

	if(fpga_channel_reconfig && fpga->configured()) {

		int channelconfig = 0;
		for(channel_t i=0; i<channels; i++) {
			channelconfig |= (chans[i]>0) << i;
		}

		fpga->setChannels(channelconfig);
		fpga_channel_reconfig = 0;
	}

	channel_mutex.unlock();
}

void _TimeTagger::read(_Worker* w) {

	while(!fpga->configured()) {
		configureFpga();
		if(!fpga->configured()) {
			// send something usefull, even if not connected.
			// otherwise read_mutex will deadlock on dac_write

			// send empty taglist
			w->ibuffer.clear();

			std::this_thread::sleep_for(std::chrono::milliseconds(1));
			return;
		}
	}

	// Channel reconfig
	configureChannel();
	w->ibuffer.resize(buffersize);
	int len = fpga->read(w->ibuffer.data(), buffersize * sizeof(NativeTag));

	if (len != buffersize * sizeof(NativeTag)) {
		//error("wanted %d, got %d bytes from tagger", ibuffersize, len);
	}

	if (len < 0) {
		// send an overflow
		w->ibuffer.resize(1);
		w->ibuffer[0].hex = 0;
		w->ibuffer[0].overflow = 1;
	} else if (len != buffersize * sizeof(NativeTag)) {
		w->ibuffer.resize(len / sizeof(NativeTag));
	}
}

void _TimeTagger::convert(_Worker *w) {

	update_distribution();
	w->obuffer.clear();
	timestamp_t now = rollover;

	//for (auto tag : w->ibuffer) {
	for (std::vector<NativeTag>::iterator it=w->ibuffer.begin(); it!=w->ibuffer.end(); ++it) {
		NativeTag &tag=*it;

        if (!tag.hex) continue;
		//printf("hex: %x, time: %d, dist: %d, chan: %d, usused: %d, edge: %d, overflow: %d, rollover: %d\n",
		//       tag.hex, tag.time, tag.dist, tag.chan, tag.unused, tag.edge, tag.overflow, tag.rollover);

		// overflow
		if (tag.overflow) {
			// We can't know if there are missed tags which must be before the already queued ones, so just skip them
			while(!tag_queue.empty())
				tag_queue.pop();

			// make sure the next tags are beyond now
			rollover += picoseconds<<16;

			Tag t;
			t.chan = CHANNEL_INVALID;
			t.time = rollover + tag.time * picoseconds;
			t.overflow = 1;
			w->obuffer.push_back(t);
        }

        // rollover
        if (tag.rollover)
            rollover += picoseconds<<16;

        // now, there won't be any edges before this time
        now = rollover + tag.time * picoseconds;

		// data
		if (tag.edge) {
			distribution_counts[tag.chan][tag.dist]++;
			Tag t;
			t.chan = tag.chan;
			t.time = now + distribution_ps[tag.chan][tag.dist] + tag_delay[tag.chan] + default_channel_delay[tag.chan];
			if(distribute_tags) {
				uint32_t delta = distribution_ps[tag.chan][tag.dist+1] - distribution_ps[tag.chan][tag.dist];
				if(delta) {
					t.time += prbs.get() % delta;
				}
			}
			t.overflow = 0;
			tag_queue.push(t);
		}

		// flush all queued tags
		while(!tag_queue.empty() && now - tag_queue.top().time > 0) {
			w->obuffer.push_back(tag_queue.top());
			tag_queue.pop();
		}
	}
	w->time = now;
}


void _TimeTagger::setLineDelay(channel_t channel, timestamp_t time) {
	convert_mutex.lock();
	tag_delay[channel] = time;
	convert_mutex.unlock();
}

timestamp_t _TimeTagger::getLineDelay(channel_t channel) {
	return tag_delay[channel];
}

std::vector<timestamp_t> _TimeTagger::getLineDelay() {
	std::vector<timestamp_t> ret;
	for (size_t n=0; n<sizeof(tag_delay)/sizeof(*tag_delay); ++n)
		ret.push_back(tag_delay[n]);
	return ret;
}


void _TimeTagger::queue_init() {
	//for (auto & elem : tag_delay) {
	for (size_t n=0; n<sizeof(tag_delay) / sizeof(*tag_delay); ++n) {
		tag_delay[n]=0;
	}
}

void _TimeTagger::getDistributionCount(channel_t chan, long long **ARGOUTVIEWM_ARRAY1, int *DIM1) {
	convert_mutex.lock();

	*ARGOUTVIEWM_ARRAY1 = new long long[distribution];
	*DIM1 = distribution;

	for(int i=0; i<distribution && *ARGOUTVIEWM_ARRAY1; i++) {
		(*ARGOUTVIEWM_ARRAY1)[i] = distribution_counts[chan][i];
	}
	convert_mutex.unlock();
}

void _TimeTagger::getDistributionPSecs(channel_t chan, timestamp_t **ARGOUTVIEWM_ARRAY1, int *DIM1) {
	convert_mutex.lock();

	*ARGOUTVIEWM_ARRAY1 = new timestamp_t[distribution];
	*DIM1 = distribution;

	for(int i=0; i<distribution && *ARGOUTVIEWM_ARRAY1; i++) {
		(*ARGOUTVIEWM_ARRAY1)[i] = distribution_ps[chan][i];
	}
	convert_mutex.unlock();
}

channel_t _TimeTagger::getChannels()
{
    return channels;
}

timestamp_t _TimeTagger::getPsPerClock()
{
    return picoseconds;
}

void _TimeTagger::sync() {
	// TODO: flush the sd-ram fifo on the FPGA

	// We aren't allowed to call read_mutex.lock as this may stall forever.
	// So we just wait for the fpga_mutex.
	fpga_mutex.lock();
	convert_mutex.lock();
	fpga_mutex.unlock();
	convert_mutex.unlock();
}


void _TimeTagger::iter(_Worker* w) {


	// Stage one: read
	read_mutex.lock();
	fpga_mutex.lock();
	read(w);
	fpga_mutex.unlock();

	// Stage two: convert
	convert_mutex.lock();
	read_mutex.unlock();
	convert(w);

	// Stage three: iterators
	std::mutex* mutex = &convert_mutex;
	_Iter** next = &iterators;

	while(*next) {
		(*next)->getMutex()->lock();
		_Iter* tmp = (*next);

		if(!tmp->iter) {
			*next = tmp->next;
			tmp->getMutex()->unlock();
			delete tmp;
		} else {
			mutex->unlock();
			mutex = tmp->getMutex();

			_next(w, tmp->iter);

			next = &tmp->next;
		}
	}

	// after Stage three, unlock the mutex
	mutex->unlock();
}

void _TimeTagger::_next(_Worker* w, _Iterator *i) {
	Tag *buffer = w->obuffer.data();
	int count = w->obuffer.size();
	timestamp_t time = w->time;

	if (i->state==STATE_IDLE) {
		if (i->start_trigger_channel != CHANNEL_INVALID) {
			for ( ;count>0; --count, buffer++) {
				if (buffer->chan==i->start_trigger_channel) {
					break;
				}
			}
		}
	}
	bool stop_after_next=false;
	if(i->state==STATE_RUNNING && (i->stop_trigger_channel != CHANNEL_INVALID)) {
		int cnt=0;
		Tag *ptag=buffer;
		for ( ; cnt<count; ++cnt, ++ptag) {
			if (ptag->chan==i->stop_trigger_channel) {
				count=cnt;
				stop_after_next=true;
				break;
			}
		}
	}
	if(i->state==STATE_RUNNING) {
		if (i->_first_timestamp==0 && count>0)
			i->_first_timestamp=buffer->time;

		i->next(w, buffer, count, time);

		if (count>0)
			i->_last_timestamp=buffer[count-1].time;

		if (stop_after_next)
			i->state=STATE_STOPPED;
	}
}

/**
 * lvl0: Fehlerhafte Aufrufe
 * lvl1: Fehler in der Komunikation mit dem FPGA
 * lvl2: Fehler in der Speicherverwaltung
 * lvl3: Fehler im Buffer
 */
void _TimeTagger::warning(int lvl, const char* msg) {
	switch (lvl) {
	case 0:	error(msg); break;
	case 1: error(msg); break;
	case 2: error(msg); break;
	case 3: error(msg); break;
	default: break;
	}
}

void _TimeTagger::update_distribution() {
	for(channel_t c=0; c<channels; c++) {
		timestamp_t* d = distribution_ps[c];
		timestamp_t sum = 0;

		d[0] = 0;
		for(int i=1; i<distribution+1; i++) {
			d[i] = d[i-1]+distribution_counts[c][i-1];
		}
		sum = d[distribution];

		for(int i=0; i<distribution; i++) {
			if (sum > 100000) {
				if(distribute_tags)
					// we add the distribution later, so just use the starting point
					d[i] = picoseconds * d[i] / sum;
				else
					// set the tag in the middle of the bin
					d[i] = picoseconds * (d[i]+distribution_counts[c][i]/2) / sum;
			} else {
				d[i] = ((i)*picoseconds)/distribution;
			}
		}
		d[distribution] = picoseconds;

		if(sum > 1000000) {
			for(int i=0; i<distribution; i++)
				distribution_counts[c][i] = (timestamp_t)(distribution_counts[c][i] * 0.9);
		}
	}
}

void _TimeTagger::setTestsignal(channel_t chan, bool enable)
{
	fpga_mutex.lock();
	calibration_enabled = (calibration_enabled & ~(1 << chan)) | enable << chan;
	updateCalibrations();
	fpga_mutex.unlock();
}

bool _TimeTagger::getTestsignal(channel_t chan)
{
	return calibration_enabled & (1 << chan);
}

void _TimeTagger::updateCalibrations()
{
	fpga->setWireIn(ADDR_CALIBRATION, calibration_enabled);
	fpga->UpdateWireIns();
}

void _TimeTagger::autoCalibration(bool verbose)
{
	for(channel_t i=0; i<8; i++) {
		setTestsignal(i, true);
		registerChannel(i);
		registerChannel(i+8);
	}

	std::this_thread::sleep_for(std::chrono::milliseconds(250));
	convert_mutex.lock();

	for(channel_t i=0; i<8; i++) {
		double res[2] = {-1, -1};
		for(int down=0; down<2; down++) {
			long long sum3 = 0;
			long long sum = 0;
			for(int x=0; x<distribution; x++) {
				long long c = distribution_counts[i + 8 * down][x];
				sum += c;
				sum3 += c*c*c;
			}
			if(sum != 0) {
				res[down] = sqrt(sum3 / (sum * 12)) / sum * picoseconds;
			}
		}
		if(verbose)
			printf("Channel %i, up %.1f ps, down %.1f ps.\n", i, res[0], res[1]);
	}
	convert_mutex.unlock();

	for(int i=0; i<8; i++) {
		unregisterChannel(i);
		unregisterChannel(i+8);
		setTestsignal(i, false);
	}

	sync();

	// We have to wait until all false events are gone
	std::this_thread::sleep_for(std::chrono::milliseconds(250));
}

void _Worker::start(_Worker *w) {
	w->work();
}

_Worker::_Worker(_TimeTagger* t) :
	time(0), run(1), tagger(t),
	thread((void (*)(void*))_Worker::start, (void*)this) {}

_Worker::~_Worker() {
	run = 0;
	thread.join();
}

void _Worker::work() {
	while(run)
		tagger->iter(this);
}


_Iterator::_Iterator(TimeTagger* tagger) {
	state = STATE_STOPPED;
	_first_timestamp = _last_timestamp = 0;

	this->tagger = tagger;
    chans.resize(tagger->getChannels());

    // for(auto & elem : chans))))
    //	elem = 0;
	for(size_t n=0; n<chans.size(); ++n)
		chans[n]=0;

	iter = tagger->addIterator(this);

	start_trigger_channel = stop_trigger_channel = CHANNEL_INVALID;
	start_trigger_time = stop_trigger_time = CHANNEL_INVALID;

	_overflows=0;

}

_Iterator::~_Iterator() {
	stop();

	for(channel_t i=0; i<chans.size(); i++)
		unregisterChannel(i);

	lock();
	iter->iter = 0; //nullptr;
	unlock();
}


void _Iterator::registerChannel(channel_t chan) {
	if(chan == CHANNEL_INVALID) return;

	if(chan >= chans.size()) {
		warning("Warning, tried to register unknown channel %d", chan);
		return;
	}
	if(!chans[chan] && tagger) {
		chans[chan] = 1;
		tagger->registerChannel(chan);
	}
}

void _Iterator::unregisterChannel(channel_t chan) {
	if(chan == CHANNEL_INVALID) return;

	if(chan >= chans.size()) {
		warning("Warning, tried to unregister unknown channel %d",chan);
		return;
	}

	if(chans[chan] && tagger) {
		chans[chan] = 0;
		tagger->unregisterChannel(chan);
	}
}

void _Iterator::start() {
	tagger->sync();

	lock();

	_overflows=0;
	_first_timestamp=0;
	_last_timestamp=0;

	if (this->start_trigger_channel == CHANNEL_INVALID && this->start_trigger_time == CHANNEL_INVALID)
		state = STATE_RUNNING;
	else {
		state = STATE_IDLE;
	}
	unlock();
}

void _Iterator::stop() {
	lock();
	state = STATE_STOPPED;
	unlock();
}

enum running_state _Iterator::status() {
	return state;
}

void _Iterator::signal_overflow() {
	//TODO: possibly set to error if a certain threshold is reached
	_overflows++;
}

long long _Iterator::getOverflows() {
	return _overflows;
}

timestamp_t _Iterator::getDuration() {
	return this->_last_timestamp-this->_first_timestamp;
}

void _Iterator::lock() {
	iter->getMutex()->lock();
}

void _Iterator::unlock() {
	iter->getMutex()->unlock();
}

void _Iterator::next(_Worker *w, Tag* list, int count, timestamp_t time) {
	next(list, count, time);
}
