/*
    backend for TimeTagger, an OpalKelly based single photon counting library
    Copyright (C) 2011  Markus Wick <wickmarkus@web.de>
    Copyright (C) 2013  Mario GLiewe <mag@bonitaposse.net>

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

#include <assert.h>
#include <string.h>
#include <vector>

#include "TimeTagger.h"

#ifndef TT_COUNTER_IMPL_CPP
#define TT_COUNTER_IMPL_CPP

/**
 * \ingroup ITERATOR
 *
 * \brief a simple counter on one or more channels
 *
 * Counter with fixed binwidth and circular buffer output. This class
 * is suitable to generate a time trace of the count rate on one or more channels.
 * The thread repeatedly counts clicks on a single channel over a
 * given time interval and stores the results in a two-dimensional array.
 * The array is treated as a circular buffer. I.e., once the array is
 * full, each new value shifts all previous values one element to the left.
 *
 */
class Counter : public _Iterator {
public:
	/**
	 * \brief construct a counter
	 *
	 * \param tagger		reference to a TimeTagger
	 * \param _channels		channels to count on
	 * \param _binwidth     counts are accumulated for binwidth picoseconds
	 * \param _n_values     number of counter n_values stored (for each channel)
	 */
	Counter(TimeTagger* tagger, std::vector<channel_t> _channels, timestamp_t _binwidth = 1000000000000LL, int _n_values = 1) : _Iterator(tagger), channels(_channels) {
		binwidth = _binwidth;
		n_values = _n_values;

		data = new int[(n_values + 1) * channels.size()];

		clear();

		for(channel_t i=0; i<channels.size(); i++)
			registerChannel(channels[i]);

		start();
	}

	virtual ~Counter() {
		stop();
		delete [] data;
	}

	virtual void clear() {
		lock();
		zero();
		unlock();
	}

	/**
	 * \brief get counts
	 *
	 * the counts are copied to a newly allocated allocated memory, an the
	 * pointer to this location is returned.
	 *
	 */
	GET_DATA_2D(getData, int, array_out) {
		assert(data);
		int *arr = array_out(channels.size(), n_values);
		assert(arr);

		lock();

		for (size_t c=0; c<channels.size(); c++)
			for (int i=0; i<n_values; i++) {
				arr[i + n_values * c] = data[(i + pos + 1) % (n_values+1) + (n_values + 1) * c];
			}

		unlock();
	}

	GET_DATA_1D(getIndex, timestamp_t, array_out) {
		timestamp_t* arr = array_out(n_values);

		for (int i=0; i<n_values; i++) {
			arr[i] = i * binwidth;
		}
	}

protected:
	virtual void next(Tag* list, int count, timestamp_t time) {
		for(int i=0; i<count; i++) {
			if(list[i].overflow) {
				starttime = -1;
				pos = 0;
				signal_overflow();

			} else for(size_t c=0; c<channels.size(); c++)
				if(list[i].chan == channels[c]) {
				// erster durchlauf nach clean
				if(starttime == -1) starttime = list[i].time;

				// lange kein paket mehr gesehen
				if(list[i].time >= starttime + binwidth*(n_values+1)) {
					zero();
					starttime = list[i].time;
				}

				// nicht mehr im aktuellen bin
				while(list[i].time >= starttime + binwidth) {
					starttime += binwidth;
					pos = (pos+1)%(n_values+1);
					for(size_t c2=0; c2<channels.size(); c2++)
						data[pos + (n_values + 1)*c2] = 0;
				}

				data[pos + (n_values + 1)*c]++;
			}
		}
		// lange kein paket mehr gesehen
		if(time >= starttime + binwidth*(n_values+1)) {
			zero();
			starttime = time;
		}
	}

private:
	void zero() {
		starttime = -1;
		pos = 0;
		for(size_t c=0; c<channels.size(); c++)
			for(int i=0; i<n_values+1; i++)
				data[i + (n_values + 1)*c] = 0;
	}

	int *data;

	const std::vector<channel_t> channels;

	timestamp_t binwidth;

	int n_values;

	int pos;

	timestamp_t starttime;
};

/**
 * \ingroup ITERATOR
 * \brief count rate on one or more channels
 *
 * Measures the average count rate on one or more channels. Specifically, it counts
 * incoming clicks and determines the time between the initial click and the latest click.
 * The number of clicks divided by the time corresponds to the average countrate
 * since the initial click.
 *
 */
class Countrate : public _Iterator {
public:

	/**
	 * \brief constructor of Countrate
	 *
	 * @param tagger		reference to a TimeTagger
	 * @param _channels		the channels to count on
	 */
	Countrate(TimeTagger* tagger, std::vector<channel_t> _channels) : _Iterator(tagger), channels(_channels), flanken(_channels.size()) {
		clear();

		for (size_t c=0; c<channels.size(); c++)
			registerChannel(channels[c]);
		start();
	}

	virtual ~Countrate() {
		stop();
	}

	/**
	 * \brief get the count rates
	 *
	 *
	 * the count rates are copied to a newly allocated allocated memory, an the
	 * pointer to this location is returned.
	 *
	 */
	GET_DATA_1D(getData, double, array_out) {
		double *arr = array_out(channels.size());

		lock();
		for (size_t c=0; c<channels.size(); c++)
			if(now == startzeit)
				arr[c] = 0;
			else
				arr[c] = 1.0e12*double(flanken[c])/double(now-startzeit);
		unlock();
	}

	/**
	 * \brief clear internal state
	 *
	 */
	virtual void clear() {
		lock();
		for (size_t c=0; c<channels.size(); c++)
			flanken[c] = 0;
		startzeit = 0;
		now = 0;
		unlock();
	}

protected:
	virtual void next(Tag* list, int count, timestamp_t time) {
		for(int i=0; i<count; i++) {
			if(list[i].overflow) {
				startzeit = 0;
				for (size_t c=0; c<channels.size(); c++)
					flanken[c] = 0;
				signal_overflow();
			} else {
				for (size_t c=0; c<channels.size(); c++) {
					if(list[i].chan == channels[c]) {
						if(!startzeit) startzeit = list[i].time;
						flanken[c]++;
					}
				}
			}
		}
		now = time;
		if(!startzeit) startzeit = time;
	}
private:
	const std::vector<channel_t> channels;

	timestamp_t now;
	timestamp_t startzeit;
	std::vector<timestamp_t> flanken;
};

#endif // #ifndef TT_COUNTER_IMPL_CPP
