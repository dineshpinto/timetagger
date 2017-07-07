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


#ifndef TT_STARTSTOP_IMPL_CPP
#define TT_STARTSTOP_IMPL_CPP

#include <map>
#include "TimeTagger.h"

/**
 * @ingroup ITERATOR
 *
 * \brief simple start-stop measurement
 *
 * This class performs a start-stop measurement between two channels
 * and stores the time differences in a histogram. The histogram resolution
 * is specified beforehand (binwidth) but the histogram range is unlimited.
 * It is adapted to the largest time difference that was detected. Thus
 * all pairs of subsequent clicks are registered.
 *
 * Be aware, on long-running measurements this may considerably slow down
 * system performance and even crash the system entirely when attached to an
 * unsuitable signal source.
 *
 */
class StartStop : public _Iterator {
public:

	/**
	 * \brief constructor of StartStop
	 *
	 * @param tagger                reference to a TimeTagger
	 * @param _click_channel         channel for stop clicks
	 * @param _start_channel         channel for start clicks
	 * @param _binwidth              width of one histogram bin in ps
	 */
	StartStop(TimeTagger* tagger, channel_t _click_channel, channel_t _start_channel=CHANNEL_INVALID, timestamp_t _binwidth=1000) : _Iterator(tagger) {
		channel  = _click_channel;
		channel2 = (_start_channel==CHANNEL_INVALID)?_click_channel:_start_channel;
		binwidth = _binwidth;
		registerChannel(channel);
		registerChannel(channel2);
		clear();
		start();
	}

	virtual ~StartStop() {
		stop();
	}

	virtual void start() {
		lock();
		last = 0;
		unlock();
		_Iterator::start();
	}

	/**
	 */
	GET_DATA_2D(getData, timestamp_t, array_out) {
		lock();
		int s = data.size();
		timestamp_t *arr = array_out(s, 2);
		std::map<timestamp_t,size_t>::iterator it;
		int i=0;

		for ( it=data.begin() ; it != data.end(); it++ ) {
			arr[i++] = it->first*binwidth;
			arr[i++] = it->second;
		}

		unlock();
	}

	/**
	 * \brief returns the number of start clicks
         *
         * Useful for normalization.
         *
	 */
	size_t getCounts() {
		return counts;
	}

	virtual void clear() {
		lock();
		last = 0;
		data.clear();
		counts=0;
		unlock();
	}

protected:
	virtual void next(Tag* list, int count, timestamp_t time) {
		for(int i = 0; i < count; i++) {
			if(list[i].overflow) {
				last = 0;
				signal_overflow();
			} else if(list[i].chan == channel && last != 0) {
				counts++;
				timestamp_t slot=(list[i].time - last)/binwidth;

				data[slot]=1;

				if(channel == channel2)
					last = list[i].time;
				else
					last = 0;
			} else if(list[i].chan == channel2)
				last = list[i].time;
		}
	}

private:
	std::map<timestamp_t,size_t> data;
	channel_t channel;
	channel_t channel2;
	timestamp_t binwidth;
	timestamp_t last;
	size_t counts;
};

#endif // #ifndef TT_STARTSTOP_IMPL_CPP
