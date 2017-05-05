/*
 * TimeTagger.cpp
 *
 *  Created on: 14.05.2013
 *      Author: mag
 */

#include "TimeTagger.h"
#include "backend.h"
#include <map>

static std::mutex timetagger_global_mutex;
static TimeTagger* last_tagger = NULL;
static std::map<TimeTagger*, int> refcount;

TimeTagger* createTimeTagger(std::string serial) {
	TimeTagger* tagger = NULL;
	timetagger_global_mutex.lock();

	// try to find a cached tagger
	if (serial != "") {
		std::map<TimeTagger*, int>::iterator it;
		for (it = refcount.begin(); it != refcount.end(); it++) {
			if (it->first->getSerial() == serial) {
				tagger = it->first;
			}
		}
	} else {
		tagger = last_tagger;
	}

	// else create a new one
	if (!tagger) {
		last_tagger = tagger = new _TimeTagger(serial);
		refcount[tagger] = 0;
	}

	refcount[tagger]++;
	timetagger_global_mutex.unlock();
	return tagger;
}

bool freeTimeTagger(TimeTagger* tagger)
{
	bool deleted = false;
	timetagger_global_mutex.lock();

	if (refcount.find(tagger) != refcount.end() && --refcount[tagger] == 0) {
		delete (_TimeTagger*)tagger;
		deleted = true;
		refcount.erase(tagger);
		if (tagger == last_tagger)
			last_tagger = NULL;
	}

	timetagger_global_mutex.unlock();
	return deleted;
}
