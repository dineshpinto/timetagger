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
 * Logger.h
 *
 *  Created on: 15.03.2013
 *      Author: mag
 */

#ifndef LOGGER_H_
#define LOGGER_H_

#include <stdio.h>
#include <stdarg.h>
#include <mutex>

/**
 * \ingroup BACKEND
 *
 * \brief static logging class
 *
 * provides a thread-safe logging mechanism.
 *
 */
class Logger {
public:
	/** logging kategories
	 */
	enum MODULE {
		APPLICATION	= 0x01, ///< application level
		BACKEND		= 0x02, ///< timetagger backend
		MODULES		= 0x08, ///< timetagger iterator
		SCPI		= 0x10, ///< scpi interface
		SCPIBIND	= 0x10, ///< scpibind interface
		HTTP		= 0x40, ///< http interface
	};

	/** logging serverities
	 */
	enum LOG_LEVEL {
		LOG_VERBOSE	= 0x20,	///< verbose information
		LOG_DEBUG	= 0x10, ///< debugging information
		LOG_WARNING	= 0x08, ///< warning message
		LOG_INFO	= 0x04, ///< informative message
		LOG_ERROR	= 0x02, ///< error message
		LOG_FATAL	= 0x01, ///< fatal error
	};

	/**
	 * \brief setup a logfile for logging
	 *
	 * Closes current logfile, and prepares new file for logging.
	 *
	 * If no logfile is opened, logging mesages will be reported to stderr.
	 *
	 * \param filename filename of logfile
	 */
	static void set_logfile(const char *filename);

	/**
	 * \brief put a message to logfile
	 *
	 * a printf()-like logging function.
	 *
	 * \param module module id
	 * \param lvl    severity id
	 * \param fmt    format string
	 */
	static void log(int module, int lvl, const char *fmt, ...);

	/**
	 * \brief put message to logfile
	 *
	 *
	 * a printf()-like logging function.
	 *
	 * \param module module id
	 * \param lvl    severity id
	 * \param fmt    format string
	 * \param ap	 accessor retrieved by va_start()
	 */
	static void vlog(int module, int lvl, const char* , va_list ap);

	/**
	 * \brief shutdown and close logfiles
	 *
	 * futher logging mesages will be reported to stderr.
	 */
	static void close();

private:
	static FILE *logfile;
	static std::mutex log_mutex;
};


#define INSTALL_LOG_FACILITY(facility) \
static void info(const char *fmt, ...) { \
	va_list ap; \
	va_start(ap, fmt); \
	Logger::vlog( facility, Logger::LOG_INFO, fmt, ap); \
} \
\
static void debug(const char *fmt, ...) { \
	va_list ap; \
	va_start(ap, fmt); \
	Logger::vlog( facility, Logger::LOG_DEBUG, fmt, ap); \
} \
\
static void error(const char *fmt, ...) { \
	va_list ap; \
	va_start(ap, fmt); \
	Logger::vlog( facility, Logger::LOG_ERROR, fmt, ap); \
} \
\
static void newer_call_me_im_stub_too(); \
static void newer_call_me_im_stub() { \
	info(""); debug(""); error(""); \
	newer_call_me_im_stub_too(); \
} \
static void newer_call_me_im_stub_too() { \
	newer_call_me_im_stub(); \
}

#endif /* LOGGER_H_ */
