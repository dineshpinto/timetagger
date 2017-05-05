/*
    backend for TimeTagger, an OpalKelly based single photon counting library
    Copyright (C) 2011  Markus Wick <wickmarkus@web.de>
    Copyright (C) 2013  Mario Gliewe <mag@bonitaposse.net>

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

#ifndef TIMETAGGER_H_
#define TIMETAGGER_H_

#include <string>
#include <vector>
#include <stdint.h>
#include <limits>
#include <functional>

class _Iter;

/*! \mainpage TimeTagger
 *
 * \brief backend for TimeTagger, an OpalKelly based single photon counting library
 *
 * \author Markus Wick <wickmarkus@web.de>
 * \author Helmut Fedder <helmut.fedder@gmail.com>
 * \author Mario Gliewe <mag@bonitaposse.net>
 *
 * TimeTagger provides an easy to use and cost effective hardware solution for
 * time-resolved single photon counting applications.
 *
 * This document describes the C++ native interface to the TimeTagger device.
 */

/**
 * \defgroup BACKEND TimeTagger backend
 * \brief the timetagger kernel
 */

/**
 * \defgroup ITERATOR base iterators
 * \brief base iterators for photon counting applications
 */

class TimeTagger;
class _Iterator;
class _TimeTagger;
class _Worker;


#define timestamp_t long long
#define channel_t unsigned int

/**
 * \brief Constant for invalid channel.
 * Magic channel_t value to indicate an invalid channel. So the iterators either
 * have to disable this channel, or to choose a default one.
 */

//static const channel_t CHANNEL_INVALID = -1u;  !! what is  unsigned(-1)  supposed to be??
static const channel_t CHANNEL_INVALID = std::numeric_limits<channel_t>::max();

/**
 * @ingroup BACKEND
 * current state of the backend.
 */
enum running_state {
	STATE_STOPPED=0,
	STATE_IDLE=1,
	STATE_RUNNING=2,
	STATE_ERROR=-1
};


/**
 * This wrapper functions will generate nice SWIG/numpy accessing functions
 */
#define GET_DATA_1D(function_name, type, argout)\
	void function_name(type **ARGOUTVIEWM_ARRAY1, int *DIM1) {\
		function_name([=](int d1) {\
			*DIM1 = d1;\
			*ARGOUTVIEWM_ARRAY1 = new type[d1];\
			return *ARGOUTVIEWM_ARRAY1;\
		});\
	}\
	void function_name(std::function<type*(int)> argout)

#define GET_DATA_2D(function_name, type, argout)\
	void function_name(type **ARGOUTVIEWM_ARRAY2, int *DIM1, int *DIM2) {\
		function_name([=](int d1, int d2) {\
			*DIM1 = d1;\
			*DIM2 = d2;\
			*ARGOUTVIEWM_ARRAY2 = new type[d1*d2];\
			return *ARGOUTVIEWM_ARRAY2;\
		});\
	}\
	void function_name(std::function<type*(int, int)> argout)

#define GET_DATA_3D(function_name, type, argout)\
	void function_name(type **ARGOUTVIEWM_ARRAY3, int *DIM1, int *DIM2, int *DIM3) {\
		function_name([=](int d1, int d2, int d3) {\
			*DIM1 = d1;\
			*DIM2 = d2;\
			*DIM3 = d3;\
			*ARGOUTVIEWM_ARRAY3 = new type[d1*d2*d3];\
			return *ARGOUTVIEWM_ARRAY3;\
		});\
	}\
	void function_name(std::function<type*(int, int, int)> argout)



/**
 * \brief default constructor factory.
 *
 * \param serial serial number of FPGA board to use. if empty, the first board found is used.
 */
TimeTagger* createTimeTagger(std::string serial="");

/**
 * \brief free a copy of a timetagger reference.
 * \param tagger the timetagger reference to free
 */
bool freeTimeTagger(TimeTagger* tagger);

/**
 * @ingroup BACKEND
 * \brief backend for the TimeTagger.
 *
 * The TimeTagger class connects to the hardware, and handles the communication over the usb.
 * There may be only one instance of the backend per physical device.
 *
 *
 */
class TimeTagger {
	friend class _Iterator;
protected:
	/**
	 * \brief abstract interface class
	 */
	TimeTagger() {}

	/**
	 * destructor
	 */
	virtual ~TimeTagger() {};
public:

	/**
	 * \brief set the trigger voltage threshold of a channel
	 *
	 * \param channel 	the channel to set
	 * \param voltage  	voltage level.. [0..1]
	 */
	virtual void setTriggerLevel(channel_t channel, double voltage) = 0;

	/**
	 * \brief get the trigger voltage threshold of a channel
	 *
	 * \param channel the channel
	 */
	virtual double getTriggerLevel(channel_t channel) = 0;

	virtual std::vector<double> getTriggerLevel() = 0;

	/**
	 * \brief set time delay on a channel
	 *
	 * When set, every event on the channel is delayed by the given delay in picoseconds.
	 * Setting larger time delays consumes, dependend on input signal, a significant
	 * amount of memory.
	 * Implementers are adviced to keep the delay below 1 micro second.
	 *
	 * \param channel  the channel to set
	 * \param delay    the delay in picoseconds
	 */
	virtual void setLineDelay(channel_t channel, timestamp_t delay) = 0;

	/**
	 * \brief get time delay of a channel
	 *
	 * see setLineDelay
	 *
	 * \param channel 	the channel
	 */
	virtual timestamp_t getLineDelay(channel_t channel) = 0;

	virtual std::vector<timestamp_t> getLineDelay() = 0;

	/**
	 * \brief enables or disables the filter on the FPGA board.
	 *
	 * The filter disables transmission of nontriggered tags on channel 7.
	 *
	 * Refer the Manual for a description of this function.
	 */
	virtual void setFilter(bool state) = 0;

	//--------------------
	//added 04.09.2015, n.abt --> for bitstream and jump address for AWG
	//--------------------

	/**
	 *
	 */
	virtual void setAWGDataOne(int data_one) = 0;
	virtual void setAWGDataTwo(int data_two) = 0;

	//--------------------

	/**
	 * \brief returns the filter state on the FPGA board
	 *
	 * The laserfilter disables transmission of nontriggered tags on channel 7.
	 *
	 * Refer the Manual for a description of this function.
	 */
	virtual bool getFilter() = 0;


	/**
	 * \brief enables or disables the normalization of the distribution.
	 *
	 * Refer the Manual for a description of this function.
	 */
	virtual void setNormalization(bool state) = 0;

	/**
	 * \brief returns the the normalization of the distribution.
	 *
	 * Refer the Manual for a description of this function.
	 */
	virtual bool getNormalization() = 0;

	/**
	 * \brief set the deadtime between two edges on the same channel.
	 *
	 * This function sets the user configureable deadtime. The requested time will
	 * be rounded to the nearest multiple of the clock time. The deadtime will also
	 * be clamped to device specific limitations.
	 *
	 * As the actual deadtime will be altered, the real value will be returned.
	 *
	 * \param chan channel to be configured
	 * \param deadtime new deadtime
	 * \return the real configured deadtime
	 */
	virtual timestamp_t setDeadtime(channel_t chan, timestamp_t deadtime) = 0;

	/**
	 * \brief register a FPGA channel.
	 *
	 * Only events on previously registered channels will be transfered over
	 * the communication channel.
	 *
	 * \param chan	the channel
	 */
	virtual void registerChannel(channel_t chan) = 0;

	/**
	 * \brief release a previously registered channel.
	 *
	 * \param chan 	the channel
	 */
	virtual void unregisterChannel(channel_t chan) = 0;

	/**
	 * \brief enable the calibration on a channel.
	 *
	 * This will connect or disconnect the channel with the on-chip uncorrelated signal generator.
	 *
	 * \param chan	the channel
	 * \param enabled	enabled / disabled flag
	 */
	virtual void setTestsignal(channel_t chan, bool enabled) = 0;

	/**
	 * \brief fetch the status of the test signal generator
	 *
	 * \param chan 	the channel
	 */
	virtual bool getTestsignal(channel_t chan) = 0;

	/**
	 * \brief runs a calibrations based on the on-chip uncorrelated signal generator.
	 *
	 * \param chan 	the channel
	 */
	virtual void autoCalibration(bool verbose = true) = 0;

	/**
	 * \brief identifies the hardware type
	 *
	 */
	virtual int getBoardModel() = 0;

	/**
	 * \brief get the current running state of the backend
	 */
	virtual enum running_state getStatus() = 0;

	/**
	 * \brief identifies the hardware by serial number
	 */
	virtual std::string getSerial() = 0;

	/**
	 * \brief get internal calibration data
	 */
	virtual void getDistributionCount(channel_t chan, long long **ARGOUTVIEWM_ARRAY1, int *DIM1) = 0;

	/**
	 * \brief get internal calibration data
	 */
	virtual void getDistributionPSecs(channel_t chan, timestamp_t **ARGOUTVIEWM_ARRAY1, int *DIM1) = 0;

	/**
	 *  \brief fetch the amount of channels
	 */
	virtual channel_t getChannels() = 0;

	/**
	 * \brief fetch the duration of each clock cycle in picoseconds
	 */
	virtual timestamp_t getPsPerClock() = 0;

	/**
	 * \brief Sync the timetagger pipeline, so that all started iterators and their enabled channels are ready
	 */
	virtual void sync() = 0;

private:
	// Used by _Iterator to add itself
	virtual _Iter *addIterator(_Iterator *it) = 0;
};


/**
 * @ingroup BACKEND
 * \brief a single event on a channel
 *
 * Channel events are passed from the backend to registered iterators
 * by the _Iterator::next() callback function.
 *
 * A Tag describes a single event on a channel.
 */
struct Tag {
	/// when set, there was an overflow on the communication channel.
	bool overflow;
	/// the channel number
	channel_t chan;
	/// the timestamp on the event, in picoseconds
	timestamp_t time;
};

/**
 * @ingroup BACKEND
 * \brief Base class for all iterators
 *
 *
 */
class _Iterator {
	friend class _TimeTagger;
private:
	_Iterator() {}		// forbid instance without backend

protected:
	/**
	 * \brief standard constructor
	 *
	 * will register with the TimeTagger backend.
	 */
	_Iterator(TimeTagger* tagger);

public:
	/**
	 * \brief destructor
	 *
	 * will stop and unregister prior finalization.
	 */
	virtual ~_Iterator();

	/**
	 * \brief start the iterator
	 *
	 * The default behavior for iterators is to start automatically on creation.
	 */
	virtual void start();

	/**
	 * \brief stop the iterator
	 *
	 * The iterator is put into the STOPPED state, but will still be registered with the backend.
	 */
	virtual void stop();

	/**
	 * \brief clear Iterator state.
	 *
	 * Each Iterator must implement the clear() method to reset
	 * its internal state.
	 * Implementers should guard actual reset code by the update lock.
	 */
	virtual void clear() {};

	/**
	 * \brief get running state
	 *
	 */
	virtual enum running_state status();

	/**
	 * \brief get overflow count
	 *
	 * Get the number of communicatiopn overflows occured since start of the iterator
	 */
	long long getOverflows();

	/**
	 * \brief  get run duration
	 *
	 * Return the time in picoseconds since the iterator was started
	 */
	timestamp_t getDuration();

protected:
	/**
	 * \brief register a channel
	 *
	 * Only channels registered by any iterator attached to a backend are delivered over the usb.
	 *
	 * \param chan	the channel
	 */
	void registerChannel(channel_t chan);

	/**
	 * \brief unregister a channel
	 *
	 * \param chan	the channel
	 */
	void unregisterChannel(channel_t chan);

	/**
	 * \brief aquire update lock
	 *
	 * All mutable operations on a iterator are guarded with an update mutex.
	 * Implementers are adviced to lock() an iterator, whenever internal state
	 * is queried or changed.
	 */
	void lock();

	/**
	 * \brief release update lock
	 *
	 * see lock()
	 */
	void unlock();

	/**
	 * \brief update iterator state
	 *
	 * Each Iterator must implement the next() method.
	 * The next() function is guarded by the update lock.
	 *
	 * The backend delivers each Tag on each registered channel
	 * to this callback function.
	 */
	virtual void next(_Worker *w, Tag* list, int count, timestamp_t time);

	/**
	 * \brief update iterator state
	 *
	 * Each Iterator must implement the next() method.
	 * The next() function is guarded by the update lock.
	 *
	 * The backend delivers each Tag on each registered channel
	 * to this callback function.
	 */
	virtual void next(Tag* list, int count, timestamp_t time) = 0;

	/**
	 * \brief signal an overflow
	 *
	 * Signals the occurence of an overflow back to the backend.
	 *
	 * The backend might set an iterator into ERROR state if to much
	 * overflows are registered.
	 */
	void signal_overflow();

protected:
	/// list of channels used by the iterator
	std::vector<int> chans;

	/// not implemented
	channel_t start_trigger_channel;

	/// not implemented
	channel_t stop_trigger_channel;

	/// not implemented
	timestamp_t start_trigger_time;

	/// not implemented
	timestamp_t stop_trigger_time;

	/// running state of the iterator
	running_state state;

	/// number of overflows occured since start
	timestamp_t _overflows;

	/// first Tag's timestamp seen by the iterator
	timestamp_t _first_timestamp;

	/// last Tag's timestamp seen by the iterator
	timestamp_t _last_timestamp;

private:
	_Iter *iter;
	TimeTagger* tagger;
};

#endif /* TIMETAGGER_H_ */
