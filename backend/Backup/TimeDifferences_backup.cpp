/*
    backend for TimeTagger, an OpalKelly based single photon counting library
    Copyright (C) 2011-2013  Markus Wick <wickmarkus@web.de>, Helmut Fedder <helmut.fedder@gmail.com>

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

#include <deque>
#include <iostream>
#include <assert.h>

#include "TimeTagger.h"


/**
 * @ingroup ITERATOR
 * \brief Accumulates the time differences between clicks on two channels in one or more histograms.
 *
 * Specifically, the thread waits for clicks on a first channel, the 'start channel', then measures the
 * time difference between a start click and all subsequent clicks on a second channel, the 'click channel',
 * and stores them in a histogram. The histogram range and resolution is specified by
 * the number of bins and the binwidth. Clicks that fall outside the histogram range are ignored.
 * Data accumulation is performed independently for all start clicks. This type of measurement
 * is frequently referred to as 'multiple start, multiple stop' measurement and corresponds to a
 * full auto- or cross-correlation measurement.
 *   The data obtained from subsequent start clicks can be accumulated into the same histogram (one-dimensional measurement)
 * or into different histograms (two-dimensional measurement). In this way you can perform more complex
 * two-dimensional time-difference measurements. Specifically, after each click on the next channel,
 * the histogram index is incremented by one and reset to zero after the last histogram has been served.
 * You can also provide an external synchronization trigger that resets the histogram index to zero.
 */
class TimeDifferences : public _Iterator {
public:
	/**
	* \brief constructor of a TimeDifferences measurement
	*
	*
	* \param tagger                 reference to a TimeTagger
	* \param _click_channel         channel that increments the count in a bin
	* \param _start_channel         channel that sets start times relative to which clicks on the click channel are measured
	* \param _next_channel          channel that increments the histogram index
	* \param _sync_channel          channel that resets the histogram index to zero
	* \param _binwidth              width of one histogram bin in ps
	* \param _n_bins                number of bins in each histogram
	* \param _n_histograms          number of histograms
	*/
	TimeDifferences(TimeTagger* tagger, channel_t _click_channel, channel_t _start_channel=CHANNEL_INVALID, channel_t _next_channel=CHANNEL_INVALID, channel_t _sync_channel=CHANNEL_INVALID, timestamp_t _binwidth=1000, int _n_bins=1000, int _n_histograms=1) : _Iterator(tagger) {

		n_bins = _n_bins;
		binwidth = _binwidth;
		n_histograms = _n_histograms;

		click_channel = _click_channel;
		start_channel = _start_channel == CHANNEL_INVALID ? click_channel : _start_channel;
		next_channel = _next_channel;
		sync_channel = _sync_channel;

		max_counts = 0;

		registerChannel(click_channel);
		registerChannel(start_channel);
		registerChannel(next_channel);
		registerChannel(sync_channel);

		data = new int[n_bins*n_histograms];
		if(!data)
			std::cout << "Warning: Konnte Speicher nicht allocieren" << std::endl;

		clear();

		start();
	}

	virtual ~TimeDifferences() {
		stop();
		if(data)
			delete [] data;
	}

	virtual void start() {
		lock();
		waiting_for_sync = sync_channel != CHANNEL_INVALID;
		pulse = next_channel == CHANNEL_INVALID ? 0 : -1;
		unlock();
		_Iterator::start();
	}

	/**
	* \brief get result data
	*
	*
	* \param ARGOUTVIEWM_ARRAY2    pointer receiving pointer to data
	* \param DIM1                  pointer receiving first dimension
	* \param DIM2                  pointer receiving second dimension
	*/
	GET_DATA_2D(getData, int, array_out) {
		if(!data) return;
		int *arr = array_out(n_histograms, n_bins);
		if(!arr)
			return;

		lock();

		for ( int i=0; i<n_bins*n_histograms; i++) {
			arr[i] = data[i];
		}

		unlock();
	}

	GET_DATA_1D(getIndex, timestamp_t, array_out) {
		timestamp_t* arr = array_out(n_bins);

		for (int i=0; i<n_bins; i++) {
			arr[i] = i * binwidth;
		}
	}

	/**
	* \brief set the number of start clicks after which acquisition shall stop
	*
	*
	* \param c             maximum number of start clicks
	*/
	void setMaxCounts(int c) {
		lock();
		max_counts = c;
		unlock();
	}

	/**
	* \brief return the number of start clicks
	*
	*
	* Useful for normalizing correlation data.
	*/
	int getCounts() {
		return counts;
	}

	/**
	* \brief return 'true' if the maximum number of start clicks has been reached
	*
	*/
	bool ready() {
		bool r;
		lock();
		r = counts >= max_counts && edge.empty() && (waiting_for_sync || pulse <= 0);
		unlock();
		return r;
	}

	virtual void clear() {
		lock();
		if(data) {
			for(int i=0; i<n_bins*n_histograms; i++) {
				data[i] = 0;
			}
		}

		waiting_for_sync = sync_channel != CHANNEL_INVALID;
		pulse = next_channel == CHANNEL_INVALID ? 0 : -1;
		counts = 0;

		edge.clear();

		unlock();
	}

protected:
	virtual void next(Tag* list, int count, timestamp_t time) {
		if(!data) return;
		for(int i = 0; i < count; i++) {
			Tag t = list[i];

			// on overflow
			if(t.overflow) {
				pulse = next_channel == CHANNEL_INVALID ? 0 : -1;
				waiting_for_sync = sync_channel != CHANNEL_INVALID;
				edge.clear();
				signal_overflow();
			}

			// on sync
			if(t.chan == sync_channel && (max_counts<=0 || counts < max_counts)) {
				pulse = next_channel == CHANNEL_INVALID ? 0 : -1;
				waiting_for_sync = 0;
				counts++;
			}

			// on next channel trigger, increase pulse
			if(t.chan == next_channel && !waiting_for_sync && (max_counts<=0 || counts < max_counts)) {
				pulse++;

				if(pulse >= n_histograms) {
					if(sync_channel != CHANNEL_INVALID) {
						waiting_for_sync = 1;
					} else {
						pulse = 0;
						counts++;
					}
				}
			}

			// on start, set edge
			if(t.chan == start_channel && !waiting_for_sync && (max_counts<=0 || counts < max_counts)) {
				if(pulse < n_histograms && pulse >= 0) {
					_PulsedEdge e;
					e.edge = t.time;
					e.pulse = pulse;
					assert(pulse >= 0 && pulse < n_histograms);
					edge.push_back(e);
				}
				while(!edge.empty() && t.time - edge.front().edge > n_bins*binwidth)
					edge.pop_front();
			}

			// on photon in histogram range
			if(t.chan == click_channel) {
				std::deque<_PulsedEdge>::iterator it;
				for(it = edge.begin(); it != edge.end(); it++) {
					timestamp_t bin = (t.time - (it->edge))/binwidth;
					if( bin < n_bins && bin>=0 ) {
						assert(it->pulse < n_histograms);
						assert(it->pulse >= 0);

						assert(it->pulse*n_bins + (t.time-(it->edge))/binwidth < n_histograms*n_bins);
						assert(it->pulse*n_bins + (t.time-(it->edge))/binwidth >= 0);

						data[it->pulse*n_bins + bin]++;
					}
				}
			}
		}
		while(!edge.empty() && time - edge.front().edge > n_bins*binwidth)
			edge.pop_front();
	}

private:
	struct _PulsedEdge {
		timestamp_t edge;
		int pulse;
	};

	channel_t click_channel, start_channel, next_channel, sync_channel;
	int n_bins, n_histograms;
	timestamp_t binwidth;

	int *data;

	std::deque<_PulsedEdge> edge;

	bool waiting_for_sync;
	int pulse, counts, max_counts;
};

/**
 * @ingroup ITERATOR
 *
 * \brief Accumulate time differences into a histogram
 *
 * This is a simple multiple start, multiple stop measurement. This is a special
 * case of the more general 'TimeDifferences' measurement.
 *    Specifically, the thread waits for clicks on a first channel, the 'start channel',
 * then measures the time difference between the last start click and all subsequent
 * clicks on a second channel, the 'click channel', and stores them in a histogram.
 * The histogram range and resolution is specified by the number of bins and the binwidth.
 * Clicks that fall outside the histogram range are ignored.
 * Data accumulation is performed independently for all start clicks. This type of measurement
 * is frequently referred to as 'multiple start, multiple stop' measurement and corresponds to a
 * full auto- or cross-correlation measurement.
 */
class Histogram {
public:
	/**
	* \brief constructor of a Histogram measurement
	*
	*
	* \param tagger                 reference to a TimeTagger
	* \param _click_channel         channel that increments the count in a bin
	* \param _start_channel         channel that sets start times relative to which clicks on the click channel are measured
	* \param _binwidth              width of one histogram bin in ps
	* \param _n_bins                number of bins in the histogram
	*/
	Histogram(TimeTagger* tagger, channel_t _click_channel, channel_t _start_channel = CHANNEL_INVALID, timestamp_t _binwidth = 1000, int _n_bins = 1000)
	: td(tagger, _click_channel,  _start_channel, CHANNEL_INVALID, CHANNEL_INVALID, _binwidth, _n_bins, 1) {}

	void start() {td.start();}
	void stop()  {td.stop();}
	void clear() {td.clear();}

	GET_DATA_1D(getData, int, array_out) {
		td.getData([&](int d1, int d2) { return array_out(d2); });
	}

	GET_DATA_1D(getIndex, timestamp_t, array_out) {
		td.getIndex(array_out);
	}

	TimeDifferences td;
};

/**
 * @ingroup ITERATOR
 *
 * \brief Fluorescence lifetime imaging
 *
 * Successively acquires n histograms (one for each pixel in the image), where
 * each histogram is determined by the number of bins and the binwidth.
 * Clicks that fall outside the histogram range are ignored.
 *
 * Fluorescence-lifetime imaging microscopy or FLIM is an imaging technique for producing an image based on
 * the differences in the exponential decay rate of the fluorescence from a fluorescent sample.
 *
 * Fluorescence lifetimes can be determined in the time domain by using a pulsed source. When a population
 * of fluorophores is excited by an ultrashort or delta pulse of light, the time-resolved fluorescence will
 * decay exponentially.
 *
 */
class Flim {
public:
	/**
	* \brief constructor of a FLIM measurement
	*
	*
	* \param tagger                 reference to a TimeTagger
	* \param _click_channel         channel that increments the count in a bin
	* \param _start_channel         channel that sets start times relative to which clicks on the click channel are measured
	* \param _next_channel          channel that increments the histogram index
	* \param _binwidth              width of one histogram bin in ps
	* \param _n_bins                number of bins in each histogram
	* \param _n_histograms          number of histograms
	*/
	Flim(TimeTagger* tagger, channel_t _click_channel, channel_t _start_channel, channel_t _next_channel, timestamp_t _binwidth = 1000, int _n_bins = 1000, int _n_histograms=1)
	: td(tagger, _click_channel, _start_channel, _next_channel, CHANNEL_INVALID, _binwidth, _n_bins, 1) {}

	void start() {td.start();}
	void stop()  {td.stop();}
	void clear() {td.clear();}

	GET_DATA_2D(getData, int, array_out) {
		td.getData(array_out);
	}

	GET_DATA_1D(getIndex, timestamp_t, array_out) {
		td.getIndex(array_out);
	}

private:
	TimeDifferences td;
};

/**
 * @ingroup ITERATOR
 *
 * \brief cross-correlation between two channels
 *
 * Accumulates time differences between clicks on two channels into
 * a histogram, where all ticks are considered both as start and stop
 * clicks and both positive and negative time differences are considered.
 * The histogram is determined by the number of bins and the binwidth, which
 * are used both for the positive and the negative histogram range (i.e.,
 * length of the histogram is 2*n_bins+1).
 *
 */
class Correlation {
public:
	/**
	* \brief constructor of a correlation measurement
	*
	*
	* \param tagger                 reference to a TimeTagger
	* \param _channel_1         	first click channel
	* \param _channel_2         	second click channel
	* \param _binwidth              width of one histogram bin in ps
	* \param _n_bins                the number of bins in the resulting histogram is 2*n_bins+1
	*/
	Correlation(TimeTagger* tagger, channel_t _channel_1, channel_t _channel_2, timestamp_t _binwidth = 1000, int _n_bins = 1000)
	: right(tagger, _channel_1, _channel_2, _binwidth, _n_bins), left(tagger, _channel_2, _channel_1, _binwidth, _n_bins), same_channel(_channel_1 == _channel_2) {}

	void start() {left.start(); right.start();}
	void stop()  {left.stop(); right.stop();}
	void clear() {left.clear(); right.clear();}

	GET_DATA_1D(getData, int, array_out) {
		int *data_l, *data_r;
		int dim_l, dim_r;

		left.getData(&data_l, &dim_l);
		right.getData(&data_r, &dim_r);

		int *out = array_out(dim_l + dim_r - 1);

		for(int i=0; i<dim_l; i++) {
			out[i] = data_l[dim_l - i - 1];
		}

		if(!same_channel)
			out[dim_l - 1] += data_r[0];

		for(int i=1; i<dim_r; i++) {
			out[i + dim_l - 1] = data_r[i];
		}

		delete [] data_l;
		delete [] data_r;
	}

	GET_DATA_1D(getIndex, timestamp_t, array_out) {
		timestamp_t *data_l, *data_r;
		int dim_l, dim_r;

		left.getIndex(&data_l, &dim_l);
		right.getIndex(&data_r, &dim_r);

		timestamp_t *out = array_out(dim_l + dim_r - 1);

		for(int i=0; i<dim_l; i++) {
			out[i] = -data_l[dim_l - i - 1];
		}

		for(int i=1; i<dim_r; i++) {
			out[i + dim_l - 1] = data_r[i];
		}

		delete [] data_l;
		delete [] data_r;
	}

private:
	Histogram right, left;
	bool same_channel;
};
