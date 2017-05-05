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
#include <queue>
#include <fstream>
#include <iostream>
#include <thread>

#include "TimeTagger.h"

/**
 * \ingroup ITERATOR
 *
 * \brief a simple event queue
 *
 * A simple Iterator, just keeping a first-in first-out queue of event timestamps.
 */
class Iterator : public _Iterator {
public:
	/**
	 * \brief standard constructor
	 *
	 * @param tagger	the backend
	 * @param chan		the channel to get events from
	 */
	Iterator(TimeTagger* tagger, channel_t chan) : _Iterator(tagger) {
		registerChannel(chan);
		channel = chan;
		clear();
		start();
	}

	virtual ~Iterator() {
		stop();
	}

	/**
	 * \brief get next timestamp
	 *
	 * get the next timestamp from the queue.
	 */
	timestamp_t next() {
		while(1) {
			lock();
			bool empty = q.empty();
			timestamp_t wert = 0;
			if(!empty) {
				wert = q.front();
				q.pop();
			}
			unlock();

			if(!empty)
				return wert;

			std::this_thread::sleep_for(std::chrono::milliseconds(10));
		}
	}

	/**
	 * \brief get queue size
	 */
	int size() {
		lock();
		int wert = q.size();
		unlock();
		return wert;
	}

	/**
	 * \brief clear the queue
	 *
	 */
	virtual void clear() {
		lock();
		q = std::queue<timestamp_t>();
		unlock();
	}

protected:
	virtual void next(Tag* list, int count, timestamp_t time) {
		for(int i=0; i<count; i++) {
			if(list[i].overflow) {
				signal_overflow();
				// TODO: push StopIteration
			} else if(list[i].chan == channel)
				q.push(list[i].time);
		}
	}

private:
	std::queue<timestamp_t> q;
	channel_t channel;
};

/**
 * \brief dump all time tags to a file
 *
 */
class Dump : public _Iterator {
public:
	/**
	 * \ingroup ITERATOR
	 *
	 * \brief constructor of a Dump thread
	 *
	 * @param tagger			reference to a TimeTagger
	 * @param _filename			name of the file to dump to
	 * @param _max_tags			stop after this number of tags has been dumped
	 */
	Dump(TimeTagger* tagger, std::string _filename, size_t _max_tags=1000000) : _Iterator(tagger), max_count(_max_tags) {
		filename = _filename;
		clear();
		start();
	}

	void stop() {
		output_file.close();
		_Iterator::stop();
	}

	/**
	 * \brief tbd
	 *
	 */
	~Dump() {
		stop();
	}

	/**
	 * \brief tbd
	 *
	 */
	virtual void clear() {
		lock();
		output_file.close();
		count=0;
		output_file.open(filename.c_str(), std::ios::out | std::ios::trunc | std::ios::binary);
		unlock();
	}

protected:
	virtual void next(Tag* list, int cnt, timestamp_t time) {
		output_file.write( (char*)list, cnt * sizeof(Tag));
		count+=cnt;
		if (count>max_count) {
			// can't use stop() due to locking
			output_file.close();
			state=STATE_STOPPED;
		}
	}

private:
	std::ofstream output_file;
	std::string filename;
	size_t count, max_count;
};
