/*
    backend for TimeTagger, an OpalKelly based single photon counting library
    Copyright (C) 2011  Markus Wick <wickmarkus@web.de>
    Copyright (C) 2011  Mario GLiewe<mag@bonitaposse.net>

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

/*
 * HWClock.cpp
 *
 *  Created on: 14.03.2013
 *      Author: mag
 */

#include <limits>
#include <stdio.h>
#include <thread>


#ifdef _MSC_VER
#pragma warning( disable : 4996)
#endif

#ifdef WIN32
		#define WIN32_LEAN_AND_MEAN
        #include <windows.h>
        #include <time.h>

        #if defined(_MSC_VER) || defined(_MSC_EXTENSIONS)
                #define DELTA_EPOCH_IN_MICROSECS  11644473600000000Ui64
        #else
                #define DELTA_EPOCH_IN_MICROSECS  11644473600000000ULL
        #endif
#else
	#include <time.h>
	#include <sys/time.h>
#endif

#include "HWClock.h"

static const int USECS_PER_SEC=1000000;
static const int USECS_PER_MSEC=1000;

#ifndef MAXINT
static const int MAXINT=std::numeric_limits<int>::max();
#endif

HWClock::HWClock() {
#ifdef WIN32

	// windows doesn't support gettimeofday.
	// the following code is shameless stolen from:
	// http://social.msdn.microsoft.com/Forums/en-US/vcgeneral/thread/430449b3-f6dd-4e18-84de-eebd26a8d668
	// http://openasthra.com/wp-content/uploads/gettimeofday.c

	FILETIME ft;
    unsigned __int64 tmpres = 0;

    GetSystemTimeAsFileTime(&ft);

    tmpres |= ft.dwHighDateTime;
    tmpres <<= 32;
    tmpres |= ft.dwLowDateTime;

    /*converting file time to unix epoch*/
    tmpres -= DELTA_EPOCH_IN_MICROSECS;
    tmpres /= 10;  /*convert into microseconds*/

    _secs = (long)(tmpres / 1000000UL);
    _usecs = (long)(tmpres % 1000000UL);

#else
	struct timeval tv;
	gettimeofday(&tv,0);

	_secs=tv.tv_sec;
	_usecs=tv.tv_usec;
#endif

}

HWClock::HWClock(long long usecs) {
	this->_usecs = (unsigned long) (usecs / USECS_PER_SEC);
	this->_secs = usecs % USECS_PER_SEC;
}

HWClock HWClock::now() {
	return HWClock();
}

long long HWClock::usecs() const {
	return _secs*USECS_PER_SEC + _usecs;
}

char *HWClock::timestamp(char *buffer, size_t bufsize) {
#ifdef WIN32
	time_t secs=_secs;
	struct tm *tm = gmtime(&secs);	// this is NOT threadsave!
#else
	struct tm trec;
	time_t t=_secs;
	struct tm *tm = gmtime_r(&t, &trec);
#endif

	if (buffer && bufsize>0) {
		// bufsize SHOULD be >= 6+17+1
		sprintf(buffer,"%04d-%02d-%02d %02d:%02d:%02d.%03d",
				(tm->tm_year+1900), tm->tm_mon, tm->tm_mday,
				tm->tm_hour, tm->tm_min, tm->tm_sec, (int)_usecs/USECS_PER_MSEC);
		return buffer;
	} else
		return 0;
}

long long HWClock::operator -(const HWClock &other) const {
	return usecs()-other.usecs();
}

HWClock HWClock::operator -(long long tm) const {
	long long s=usecs()-tm;
	return HWClock(s);
}

HWClock HWClock::operator +(long long tm) const {
	long long s=usecs()+tm;
	return HWClock(s);
}

HWClock &HWClock::operator -=(long long tm) {
	long long us=usecs()-tm;
	this->_usecs = (unsigned long) (us / USECS_PER_SEC);
	this->_secs = us % USECS_PER_SEC;
	return *this;
}

HWClock &HWClock::operator +=(long long tm) {
	long long us=usecs()+tm;
	this->_usecs = (unsigned long) (us / USECS_PER_SEC);
	this->_secs = us % USECS_PER_SEC;
	return *this;
}

bool HWClock::operator ==(const HWClock &other) const {
	return usecs()==other.usecs();
}
bool HWClock::operator !=(const HWClock &other) const {
	return usecs()!=other.usecs();
}

bool HWClock::operator <(const HWClock &other) const {
	return usecs()<other.usecs();
}
bool HWClock::operator <=(const HWClock &other) const {
	return usecs()<=other.usecs();
}
bool HWClock::operator >(const HWClock &other) const {
	return usecs()>other.usecs();
}
bool HWClock::operator >=(const HWClock &other) const {
	return usecs()>=other.usecs();
}


void HWClock::sleep(long long usecs) {
	// on linux, this won't work when a signal is received
	usecs /= USECS_PER_MSEC;
	while (usecs>MAXINT) {
		std::this_thread::sleep_for(std::chrono::microseconds(MAXINT));
		usecs-=MAXINT;
	}
	if (usecs>0)
		std::this_thread::sleep_for(std::chrono::microseconds(usecs));
}

