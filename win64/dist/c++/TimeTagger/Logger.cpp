/*
    backend for TimeTagger, an OpalKelly based single photon counting library
    Copyright (C) 2011  Markus Wick <wickmarkus@web.de>
    Copyright (C) 2011 Mario GLiewe<mag@bonitaposse.net>

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
 * Logger.cpp
 *
 *  Created on: 15.03.2013
 *      Author: mag
 */

#ifdef WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#endif

#ifdef _MSC_VER
#pragma warning( disable : 4996)
#endif

#include <errno.h>
#include <stdarg.h>

#include "Logger.h"
#include "HWClock.h"

void Logger::set_logfile(const char *filename) {
	if (logfile && logfile!=stderr)
		fclose(logfile);
	if (filename && *filename) {
		logfile=fopen(filename, "a");
		if (!logfile) {
			logfile=stderr;
		}
	}
}

void Logger::close() {
	if (logfile && logfile!=stderr)
		fclose(logfile);
	logfile=stdout;
}

FILE * Logger::logfile=stdout;
std::mutex Logger::log_mutex;

void Logger::vlog(int module, int lvl, const char *fmt, va_list ap) {

#ifdef WIN32
	if (lvl & Logger::LOG_FATAL) {
		char buffer[4096];
		vsnprintf(buffer, 4096, fmt, ap);

		MessageBoxA(0,
			(buffer),
			("Timetagger Backend"),
			0x10);
	}
#endif


	log_mutex.lock();
	if (logfile && logfile!=stdout) {
		HWClock now=HWClock::now();
		char buffer[HWClock::TS_BUFFER_SIZE];
		fprintf(logfile,"[%s]-[%3x]: ", now.timestamp(buffer,HWClock::TS_BUFFER_SIZE), lvl);
	}

	vfprintf(logfile, fmt, ap);
	fprintf(logfile, "\n");
	log_mutex.unlock();
}

void Logger::log(int module, int lvl, const char *fmt, ...) {
	va_list va;
	va_start(va, fmt);
	Logger::vlog(module,lvl,fmt,va);
}

