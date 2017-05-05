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

#ifndef TT_COUNTBETWEENMARKERS_IMPL_CPP
#define TT_COUNTBETWEENMARKERS_IMPL_CPP

#include <assert.h>
#include <thread>
#include "TimeTagger.h"

/**
 * \ingroup ITERATOR
 *
 * \brief a simple counter where external marker signals determine the bins
 *
 * Counter with external signals that trigger beginning and end of each counter
 * accumulation. This can be used to implement counting triggered by a pixel
 * clock and gated counting. The thread waits for the first time tag on the
 * 'begin_channel', then begins counting time tags on the 'click_channel'.
 * It ends counting when a tag on the 'end_chanel' is detected.
 *
 */

class CountBetweenMarkers : public _Iterator {
public:
	/**
	 * \brief constructor of CountBetweenMarkers
	 *
	 *
	 * @param tagger          reference to a TimeTagger
	 * @param _click_channel  channel that increases the count
	 * @param _begin_channel  channel that triggers beginning of counting and stepping to the next value
	 * @param _end_channel    channel that triggers end of counting
	 * @param _n_values       the number of counter values to be stored
	 */
	CountBetweenMarkers(TimeTagger* tagger, channel_t _click_channel, channel_t _begin_channel, channel_t _end_channel=CHANNEL_INVALID, int _n_values=1000) : _Iterator(tagger) {
		click_channel = _click_channel;
		begin_channel = _begin_channel;
		end_channel = _end_channel;
		n_values = _n_values;

		data = new int[n_values];
		binwidths = new timestamp_t[n_values];

		clear();

		registerChannel(click_channel);
		registerChannel(begin_channel);
		registerChannel(end_channel);

		start();
	}

	virtual ~CountBetweenMarkers() {
		stop();
		delete [] data;
	}

	virtual void clear() {
		lock();

		state = -1;
		stopped = true;
		start_time = 0;
		for(int i=0; i<n_values; i++) {
			data[i] = 0;
			binwidths[i] = 0;
		}

		unlock();
	}

	/**
	 * \brief tbd
	 *
	 */
	bool ready() {
		lock();
		bool wert = state >= n_values;
		unlock();

		return wert;
	}

	/**
	 * \brief tbd
	 *
	 * @param ARGOUTVIEWM_ARRAY1
	 * @param DIM1
	 */
	GET_DATA_1D(getData, int, array_out) {
		int *arr = array_out(n_values);

		lock();

		for ( int i=0; i<n_values; i++) {
			arr[i] = data[i];
		}

		unlock();
	}

	/**
	 * \brief fetches the widths of each bins
	 */
	void getBinWidths(timestamp_t **ARGOUTVIEWM_ARRAY1, int *DIM1) {
		timestamp_t *arr = new timestamp_t[n_values];

		lock();

		for ( int i=0; i<n_values; i++) {
			arr[i] = binwidths[i];
		}

		*ARGOUTVIEWM_ARRAY1 = arr;
		*DIM1 = n_values;

		unlock();
	}

	/**
	 * \brief tbd
	 *
	 * @param ARGOUTVIEWM_ARRAY1
	 * @param DIM1
	 */
	GET_DATA_1D(getDataBlocking, int, array_out){
		while(!ready())
			std::this_thread::sleep_for(std::chrono::milliseconds(10));

		getData(array_out);
	}

protected:
	virtual void next(Tag* list, int count, timestamp_t time) {
		for(int i=0; i<count; i++) {
			if (list[i].overflow) {
				signal_overflow();
			} else {
				if(list[i].chan == begin_channel) {
					if (state >= 0 && state < n_values && !stopped) {
						binwidths[state] = list[i].time - start_time;
					}
					state++;
					stopped = false;
					start_time = list[i].time;
				} else if(list[i].chan == end_channel)
					stopped = true;
					if (state >=0 && state < n_values)
						binwidths[state] = list[i].time - start_time;

				if(list[i].chan == click_channel && state >= 0 && state < n_values && !stopped) {
					data[state]++;
				}
			}
		}
	}

private:
	int *data;
	timestamp_t *binwidths;

	channel_t click_channel;
	channel_t begin_channel;
	channel_t end_channel;
	int n_values;

	int state;
	bool stopped;
	timestamp_t start_time;
};

#endif // #ifndef TT_COUNTBETWEENMARKERS_IMPL_CPP

