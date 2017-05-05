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
 * HWClock.h
 *
 *  Created on: 14.03.2013
 *      Author: mag
 */

#ifndef HWCLOCK_H_
#define HWCLOCK_H_

#ifndef _MSC_VER
#include <unistd.h>
#endif

/**
 * \ingroup BACKEND
 *
 * \brief Access to Hardware RTC.
 *
 * When interpreted as an absolute time value, it represents the number of microseconds
 * elapsed since 00:00:00 on January 1, 1970, Coordinated Universal Time (UTC).
 */
class HWClock {
public:

	/// \brief minimum buffer size used for HWClock::timestamp()
	static const int TS_BUFFER_SIZE=24;

	/// \brief seconds elapsed till epoch
	unsigned long _secs;

	/// \brief micro seconds elapsed in this second
	unsigned long _usecs;

	/**
	 * \brief empty constructor
	 *
	 * initialized by current time.
	 */
	HWClock();

	/**
	 * \brief initialized constructor
	 *
	 * initialise with given values
	 */
	HWClock(long long usecs);

	/**
	 * \brief create a timestamp, suitable for logging.
	 *
	 * buffer *must* be at least 24bytes wide.
	 */
	char *timestamp(char *buffer, size_t bufsize);

	/**
	 * \brief return usecs till epoch
	 */
	long long usecs() const;

	/** return time difference in microseconds
	 */
	long long operator -(const HWClock &) const;

	/**
	 * \brief advance time for given microseconds
	 */
	HWClock operator +(long long) const;

	/**
	 * \brief decrease time for given microseconds
	 */
	HWClock operator -(long long) const;

	/**
	 * \brief advance time for given microseconds inplace
	 */
	HWClock &operator +=(long long);

	/**
	 * \brief decrease time for given microseconds inplace
	 */
	HWClock &operator -=(long long);

	/**
	 * \brief compare times
	 */
	bool operator ==(const HWClock &other) const;

	/**
	 * \brief compare times
	 */
	bool operator !=(const HWClock &other) const;

	/**
	 * \brief compare times
	 */
	bool operator >=(const HWClock &other) const;

	/**
	 * \brief compare times
	 */
	bool operator <=(const HWClock &other) const;

	/**
	 * \brief compare times
	 */
	bool operator <(const HWClock &other) const;

	/**
	 * \brief compare times
	 */
	bool operator >(const HWClock &other) const;

	/**
	 * \brief sleep for given microseconds.
	 *
	 * this will suspend current task for the given period.
	 */
	static void sleep(long long);

	/**
	 * \brief get current time
	 */
	static HWClock now();
};

#endif /* HWCLOCK_H_ */
