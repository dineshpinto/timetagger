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

#include "Thread.h"



#ifdef THREAD_USE_PTHREAD
	#include <pthread.h>
	#include <unistd.h>
#elif defined THREAD_USE_BOOST
	#include <boost/thread.hpp>
	#include <boost/date_time/posix_time/posix_time.hpp>
#elif defined THREAD_USE_WINDOWS
	#include <windows.h>
	#include <process.h>
#endif

current_mutex::current_mutex() {
#ifdef THREAD_USE_PTHREAD
	m = new pthread_mutex_t();
	pthread_mutex_init((pthread_mutex_t*)m, 0);
#elif defined THREAD_USE_BOOST
	m = (void*)new boost::mutex();
#elif defined THREAD_USE_WINDOWS
	m = CreateMutex(0, 0, 0);
#endif
}

current_mutex::~current_mutex() {
	if(m) {
#ifdef THREAD_USE_PTHREAD
		pthread_mutex_destroy((pthread_mutex_t*)m);
		delete (pthread_mutex_t*)m;
#elif defined THREAD_USE_BOOST
		delete (boost::mutex*)m;
#elif defined THREAD_USE_WINDOWS
		CloseHandle(m);
#endif
	}
}

void current_mutex::lock() {
#ifdef THREAD_USE_PTHREAD
	pthread_mutex_lock((pthread_mutex_t*)m);
#elif defined THREAD_USE_BOOST
	((boost::mutex*)m)->lock();
#elif defined THREAD_USE_WINDOWS
	WaitForSingleObject(m, INFINITE);
#endif
}

void current_mutex::unlock() {
#ifdef THREAD_USE_PTHREAD
	pthread_mutex_unlock((pthread_mutex_t*)m);
#elif defined THREAD_USE_BOOST
	((boost::mutex*)m)->unlock();
#elif defined THREAD_USE_WINDOWS
	ReleaseMutex(m);
#endif
}

current_thread::current_thread(void func(void*), void* arg) : t(0)
{
#ifdef THREAD_USE_PTHREAD
	t = new pthread_t();
	pthread_create((pthread_t*)t, 0, (void* (*)(void*))func, arg);
#elif defined THREAD_USE_BOOST
	t = (void*)new boost::thread(func, arg);
#elif defined THREAD_USE_WINDOWS
	t = (HANDLE)_beginthread( func, 0, arg );
#endif
}

current_thread::current_thread() : t(0){
	
}

current_thread::~current_thread() {
	if(t) {
#ifdef THREAD_USE_PTHREAD
		delete (pthread_t*)t;
#elif defined THREAD_USE_BOOST
		delete (boost::thread*)t;
#elif defined THREAD_USE_WINDOWS
#endif
	}
}

void current_thread::join() {
#ifdef THREAD_USE_PTHREAD
	pthread_join(*(pthread_t*)t, 0);
#elif defined THREAD_USE_BOOST
	((boost::thread*)t)->join();
#elif defined THREAD_USE_WINDOWS
	WaitForSingleObject(t, INFINITE);
#endif
}

void current_sleep(int ms) {
#ifdef THREAD_USE_PTHREAD
	usleep(1000*ms);
#elif defined THREAD_USE_BOOST
	boost::this_thread::sleep(boost::posix_time::milliseconds(ms));
#elif defined THREAD_USE_WINDOWS
	Sleep(ms);
#endif
}