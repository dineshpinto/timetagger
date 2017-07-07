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

#ifndef THREAD_H
#define THREAD_H



class current_mutex {
public:
	current_mutex();
	~current_mutex();
	void lock();
	void unlock();
	
private:
	void *m;
};


class current_thread {
public:
	current_thread();
	~current_thread();
	current_thread(void func(void*), void* arg) ;
	void join();

private:
	void *t;
};

void current_sleep(int ms);

#endif